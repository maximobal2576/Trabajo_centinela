from kivy.utils import platform


class UbicacionService:
    """Ubicación en primer plano mediante el proveedor GPS o de red de Android.

    Cambios v2:
    - Intervalo reducido a 500 ms / 0.5 m para actualización continua.
    - Callback opcional ``on_location(latitud, longitud)`` invocado en cada
      nueva coordenada recibida desde Java (sin depender del polling de Kivy).
    - Referencia fuerte al Listener guardada en ``self._listener`` para evitar
      que el GC de Android descarte el objeto y bloquee las actualizaciones.
    """

    def __init__(self):
        self.activo = False
        self.ultima_ubicacion = None
        self.ultimo_error = ""
        self.on_location = None   # callable(lat, lon) opcional
        self._manager = None
        self._listener = None

    # ------------------------------------------------------------------
    # INICIAR
    # ------------------------------------------------------------------

    def iniciar(self):
        self.activo = True
        if platform != "android":
            self.ultimo_error = "El GPS se prueba desde un teléfono Android."
            return False

        from android.permissions import (
            Permission,
            check_permission,
            request_permissions,
        )

        permisos = [Permission.ACCESS_FINE_LOCATION, Permission.ACCESS_COARSE_LOCATION]
        if not any(check_permission(p) for p in permisos):
            request_permissions(permisos, self._al_resolver_permisos)
            self.ultimo_error = "Esperando autorización de ubicación."
            return False

        return self._activar_actualizaciones()

    def _al_resolver_permisos(self, _permisos, _resultados):
        if self.activo:
            self._activar_actualizaciones()

    # ------------------------------------------------------------------
    # ACTIVAR ACTUALIZACIONES
    # ------------------------------------------------------------------

    def _activar_actualizaciones(self):
        try:
            from android.permissions import Permission, check_permission
            from jnius import autoclass, PythonJavaClass, java_method

            if not (
                check_permission(Permission.ACCESS_FINE_LOCATION)
                or check_permission(Permission.ACCESS_COARSE_LOCATION)
            ):
                self.ultimo_error = "Permiso de ubicación denegado."
                return False

            servicio = self  # referencia fuerte capturada en el closure

            class Listener(PythonJavaClass):
                __javainterfaces__ = ["android/location/LocationListener"]
                __javacontext__ = "app"

                @java_method("(Landroid/location/Location;)V")
                def onLocationChanged(self, ubicacion):
                    lat = ubicacion.getLatitude()
                    lon = ubicacion.getLongitude()
                    servicio.ultima_ubicacion = (lat, lon)
                    servicio.ultimo_error = ""
                    # Disparar callback si fue registrado
                    if callable(servicio.on_location):
                        try:
                            servicio.on_location(lat, lon)
                        except Exception:
                            pass

                @java_method("(Ljava/lang/String;)V")
                def onProviderDisabled(self, _proveedor):
                    servicio.ultimo_error = "Activá la ubicación del teléfono."

                @java_method("(Ljava/lang/String;)V")
                def onProviderEnabled(self, _proveedor):
                    servicio.ultimo_error = ""

                @java_method("(Ljava/lang/String;ILandroid/os/Bundle;)V")
                def onStatusChanged(self, _proveedor, _estado, _extras):
                    pass

            Context = autoclass("android.content.Context")
            LocationManager = autoclass("android.location.LocationManager")
            PythonActivity = autoclass("org.kivy.android.PythonActivity")

            self._manager = PythonActivity.mActivity.getSystemService(
                Context.LOCATION_SERVICE
            )
            # Referencia obligatoria: evita que Java descarte el listener.
            self._listener = Listener()

            # Preferir GPS_PROVIDER; caer en NETWORK si no está disponible.
            proveedor = LocationManager.GPS_PROVIDER
            if not self._manager.isProviderEnabled(proveedor):
                proveedor = LocationManager.NETWORK_PROVIDER

            # 500 ms / 0.5 m → actualización verdaderamente continua.
            self._manager.requestLocationUpdates(
                proveedor, 500, 0.5, self._listener
            )

            # Inyectar última ubicación conocida de inmediato si existe.
            conocida = self._manager.getLastKnownLocation(proveedor)
            if conocida is not None:
                lat = conocida.getLatitude()
                lon = conocida.getLongitude()
                self.ultima_ubicacion = (lat, lon)
                if callable(self.on_location):
                    try:
                        self.on_location(lat, lon)
                    except Exception:
                        pass

            self.ultimo_error = ""
            return True

        except Exception as error:
            self.ultimo_error = f"No se pudo iniciar GPS: {error}"
            return False

    # ------------------------------------------------------------------
    # DETENER
    # ------------------------------------------------------------------

    def detener(self):
        self.activo = False
        if self._manager is not None and self._listener is not None:
            try:
                self._manager.removeUpdates(self._listener)
            except Exception:
                pass
        self._manager = None
        self._listener = None

    # ------------------------------------------------------------------
    # OBTENER UBICACIÓN
    # ------------------------------------------------------------------

    def obtener_ubicacion(self):
        if not self.activo or self.ultima_ubicacion is None:
            return None
        latitud, longitud = self.ultima_ubicacion
        return {"latitud": latitud, "longitud": longitud}

    # ------------------------------------------------------------------
    # LECTURA PUNTUAL (sin seguimiento continuo)
    # ------------------------------------------------------------------

    @staticmethod
    def obtener_ubicacion_actual():
        """Lee una ubicación reciente sin iniciar un seguimiento continuo."""
        if platform != "android":
            return None
        try:
            from android.permissions import Permission, check_permission
            from jnius import autoclass

            if not (
                check_permission(Permission.ACCESS_FINE_LOCATION)
                or check_permission(Permission.ACCESS_COARSE_LOCATION)
            ):
                return None

            Context = autoclass("android.content.Context")
            LocationManager = autoclass("android.location.LocationManager")
            PythonActivity = autoclass("org.kivy.android.PythonActivity")
            manager = PythonActivity.mActivity.getSystemService(
                Context.LOCATION_SERVICE
            )
            for proveedor in (
                LocationManager.GPS_PROVIDER,
                LocationManager.NETWORK_PROVIDER,
            ):
                ubicacion = manager.getLastKnownLocation(proveedor)
                if ubicacion is not None:
                    return ubicacion.getLatitude(), ubicacion.getLongitude()
        except Exception:
            return None
        return None

    @staticmethod
    def obtener_enlace_mapa():
        ubicacion = UbicacionService.obtener_ubicacion_actual()
        if ubicacion is None:
            return None
        return f"https://www.google.com/maps?q={ubicacion[0]},{ubicacion[1]}"


# ----------------------------------------------------------------------
# SINGLETON
# ----------------------------------------------------------------------

_servicio_ubicacion = None


def get_ubicacion_service():
    """Comparte el GPS entre navegación, emergencia y guardado de rutas."""
    global _servicio_ubicacion
    if _servicio_ubicacion is None:
        _servicio_ubicacion = UbicacionService()
    return _servicio_ubicacion
