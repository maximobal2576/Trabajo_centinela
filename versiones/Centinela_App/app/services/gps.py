from kivy.utils import platform


class UbicacionService:
    """Servicio de ubicación de Centinela."""

    @staticmethod
    def obtener_ubicacion_actual():
        """
        Obtiene una ubicación reciente del teléfono.

        Devuelve:
            (latitud, longitud) si encuentra ubicación.
            None si no puede obtenerla.
        """

        if platform != "android":
            return None

        try:
            from android.permissions import Permission, check_permission
            from jnius import autoclass, PythonJavaClass, java_method

            # Verificar permisos
            tiene_fine = check_permission(
                Permission.ACCESS_FINE_LOCATION
            )
            tiene_coarse = check_permission(
                Permission.ACCESS_COARSE_LOCATION
            )

            if not (tiene_fine or tiene_coarse):
                return None

            Context = autoclass("android.content.Context")
            LocationManager = autoclass(
                "android.location.LocationManager"
            )
            PythonActivity = autoclass(
                "org.kivy.android.PythonActivity"
            )

            activity = PythonActivity.mActivity

            manager = activity.getSystemService(
                Context.LOCATION_SERVICE
            )

            # Primero intentamos GPS
            location = manager.getLastKnownLocation(
                LocationManager.GPS_PROVIDER
            )

            # Si no hay GPS, usamos red
            if location is None:
                location = manager.getLastKnownLocation(
                    LocationManager.NETWORK_PROVIDER
                )

            if location is None:
                return None

            latitud = location.getLatitude()
            longitud = location.getLongitude()

            return latitud, longitud

        except Exception as e:
            print("Error obteniendo ubicación:", e)
            return None

    @staticmethod
    def obtener_enlace_mapa():
        """
        Obtiene la ubicación y genera un enlace de Google Maps.
        """

        ubicacion = UbicacionService.obtener_ubicacion_actual()

        if ubicacion is None:
            return None

        latitud, longitud = ubicacion

        return (
            "https://www.google.com/maps?q="
            f"{latitud},{longitud}"
        )