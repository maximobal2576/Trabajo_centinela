from kivy.utils import platform


class UbicacionService:
    """Ubicación en primer plano mediante el proveedor GPS, red/datos móviles (celular),
    WiFi y Fused Provider de Android.

    Características:
    - Intervalo continuo de 500 ms / 0.5 m.
    - Soporte multicanal: GPS satelital, Network (antenas celulares/datos móviles y WiFi),
      Passive y Fused provider de Android.
    - Selección inteligente de la última ubicación conocida según el timestamp más reciente.
    - Callback reactivo on_location(lat, lon) despachado al hilo principal de Kivy.
    - Retención fuerte de referencia al Listener para evitar recolección por el GC de Java.
    """

    def __init__(self):
        self.activo = False
        self.ultima_ubicacion = None
        self.ultimo_error = ""
        self.on_location = None  # callable(lat, lon) opcional
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

            from kivy.clock import Clock

            servicio = self  # referencia fuerte capturada en el closure

            class Listener(PythonJavaClass):
                __javainterfaces__ = ["android/location/LocationListener"]
                __javacontext__ = "app"

                @java_method("(Landroid/location/Location;)V")
                def onLocationChanged(self, ubicacion):
                    lat = float(ubicacion.getLatitude())
                    lon = float(ubicacion.getLongitude())
                    servicio.ultima_ubicacion = (lat, lon)
                    servicio.ultimo_error = ""
                    # Disparar callback de forma segura en el hilo de Kivy
                    if callable(servicio.on_location):
                        cb = servicio.on_location
                        Clock.schedule_once(lambda dt: cb(lat, lon), 0)

                @java_method("(Ljava/lang/String;)V")
                def onProviderDisabled(self, _proveedor):
                    pass

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
            self._listener = Listener()

            # Obtener lista completa de proveedores disponibles en el dispositivo
            # (GPS satelital, Network para datos móviles/antenas/WiFi, Passive y Fused)
            proveedores_disponibles = []
            try:
                lista_java = self._manager.getAllProviders()
                if lista_java is not None:
                    for i in range(lista_java.size()):
                        prov_nombre = str(lista_java.get(i))
                        if prov_nombre and prov_nombre not in proveedores_disponibles:
                            proveedores_disponibles.append(prov_nombre)
            except Exception:
                pass

            # Asegurar proveedores estándar conocidos
            proveedores_base = [
                LocationManager.GPS_PROVIDER,
                LocationManager.NETWORK_PROVIDER,
            ]
            if hasattr(LocationManager, "PASSIVE_PROVIDER"):
                proveedores_base.append(LocationManager.PASSIVE_PROVIDER)
            if hasattr(LocationManager, "FUSED_PROVIDER"):
                proveedores_base.append(LocationManager.FUSED_PROVIDER)
            else:
                proveedores_base.append("fused")

            for prov in proveedores_base:
                if prov not in proveedores_disponibles:
                    proveedores_disponibles.append(prov)

            # Activar actualizaciones continuas en todos los proveedores posibles
            alguno_activo = False
            for prov in proveedores_disponibles:
                try:
                    self._manager.requestLocationUpdates(
                        prov, 500, 0.5, self._listener
                    )
                    alguno_activo = True
                except Exception:
                    pass

            # Inyectar la ubicación más reciente conocida comparando timestamps de todos los proveedores
            mejor_ubicacion = None
            mejor_timestamp = 0
            for prov in proveedores_disponibles:
                try:
                    conocida = self._manager.getLastKnownLocation(prov)
                    if conocida is not None:
                        tiempo = conocida.getTime()
                        if mejor_ubicacion is None or tiempo > mejor_timestamp:
                            mejor_ubicacion = conocida
                            mejor_timestamp = tiempo
                except Exception:
                    pass

            if mejor_ubicacion is not None:
                lat = float(mejor_ubicacion.getLatitude())
                lon = float(mejor_ubicacion.getLongitude())
                self.ultima_ubicacion = (lat, lon)
                if callable(self.on_location):
                    cb = self.on_location
                    Clock.schedule_once(lambda dt: cb(lat, lon), 0)

            if not alguno_activo and self.ultima_ubicacion is None:
                self.ultimo_error = "Activá la ubicación o datos móviles en los ajustes."
                return False

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
        """Lee la ubicación más reciente entre todos los proveedores (GPS, datos móviles, WiFi)."""
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

            proveedores = []
            try:
                lista_java = manager.getAllProviders()
                if lista_java is not None:
                    for i in range(lista_java.size()):
                        p = str(lista_java.get(i))
                        if p and p not in proveedores:
                            proveedores.append(p)
            except Exception:
                pass

            for p in (
                LocationManager.GPS_PROVIDER,
                LocationManager.NETWORK_PROVIDER,
                getattr(LocationManager, "PASSIVE_PROVIDER", "passive"),
                getattr(LocationManager, "FUSED_PROVIDER", "fused"),
            ):
                if p not in proveedores:
                    proveedores.append(p)

            mejor_ubicacion = None
            mejor_timestamp = 0
            for proveedor in proveedores:
                try:
                    ubicacion = manager.getLastKnownLocation(proveedor)
                    if ubicacion is not None:
                        tiempo = ubicacion.getTime()
                        if mejor_ubicacion is None or tiempo > mejor_timestamp:
                            mejor_ubicacion = ubicacion
                            mejor_timestamp = tiempo
                except Exception:
                    pass

            if mejor_ubicacion is not None:
                return float(mejor_ubicacion.getLatitude()), float(mejor_ubicacion.getLongitude())
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
