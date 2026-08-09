from kivy.utils import platform


class UbicacionService:
    """Obtiene la última ubicación conocida del teléfono para una alerta."""

    @staticmethod
    def obtener_enlace_mapa():
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
            activity = PythonActivity.mActivity
            manager = activity.getSystemService(Context.LOCATION_SERVICE)

            location = manager.getLastKnownLocation(LocationManager.GPS_PROVIDER)

            if location is None:
                location = manager.getLastKnownLocation(
                    LocationManager.NETWORK_PROVIDER
                )

            if location is None:
                return None

            return (
                "https://www.google.com/maps?q="
                f"{location.getLatitude()},{location.getLongitude()}"
            )
        except Exception:
            return None
