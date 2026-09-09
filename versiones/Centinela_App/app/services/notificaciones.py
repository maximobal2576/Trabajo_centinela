from kivy.utils import platform

if platform == "android":
    try:
        from jnius import autoclass
        from android.permissions import Permission, request_permissions

        Context = autoclass("android.content.Context")
        PythonActivity = autoclass("org.kivy.android.PythonActivity")
        NotificationManager = autoclass("android.app.NotificationManager")
        NotificationChannel = autoclass("android.app.NotificationChannel")
        NotificationBuilder = autoclass("android.app.Notification$Builder")
        PowerManager = autoclass("android.os.PowerManager")
    except Exception:
        PythonActivity = None
        NotificationManager = None
else:
    PythonActivity = None
    NotificationManager = None


class NotificacionService:
    """Gestiona notificaciones persistentes en la barra de estado y WakeLock para ejecución con pantalla apagada."""

    CANAL_ID = "centinela_navegacion_channel_v2"
    CANAL_NOMBRE = "Centinela Navegación"
    NOTIF_ID = 1001

    _canal_creado = False
    _wake_lock = None
    _permiso_solicitado = False

    @classmethod
    def solicitar_permisos(cls):
        if platform != "android" or cls._permiso_solicitado:
            return
        cls._permiso_solicitado = True
        try:
            from android.permissions import Permission, request_permissions
            if hasattr(Permission, "POST_NOTIFICATIONS"):
                request_permissions([Permission.POST_NOTIFICATIONS])
        except Exception:
            pass

    @classmethod
    def _asegurar_canal(cls):
        if platform != "android" or cls._canal_creado:
            return
        try:
            cls.solicitar_permisos()
            activity = PythonActivity.mActivity
            notif_service = activity.getSystemService(Context.NOTIFICATION_SERVICE)

            from android import api_version
            if api_version >= 26:
                importance = NotificationManager.IMPORTANCE_DEFAULT
                channel = NotificationChannel(cls.CANAL_ID, cls.CANAL_NOMBRE, importance)
                channel.setDescription("Indicaciones de navegación y estado de Centinela")
                channel.enableVibration(False)
                channel.setShowBadge(True)
                notif_service.createNotificationChannel(channel)

            cls._canal_creado = True
        except Exception:
            pass

    @classmethod
    def mostrar_indicacion(cls, titulo, mensaje, tiempo_estimado=None):
        """Muestra o actualiza la notificación en la barra de estado con tiempo estimado."""
        if platform != "android":
            return

        try:
            cls._asegurar_canal()
            activity = PythonActivity.mActivity
            if not activity:
                return

            notif_service = activity.getSystemService(Context.NOTIFICATION_SERVICE)
            if not notif_service:
                return

            from android import api_version
            if api_version >= 26:
                builder = NotificationBuilder(activity, cls.CANAL_ID)
            else:
                builder = NotificationBuilder(activity)

            app_info = activity.getApplicationInfo()
            builder.setSmallIcon(app_info.icon)
            builder.setContentTitle(str(titulo))
            builder.setContentText(str(mensaje))
            if tiempo_estimado:
                try:
                    builder.setSubText(f"Tiempo estimado: {tiempo_estimado}")
                except Exception:
                    pass
            builder.setOngoing(True)
            builder.setOnlyAlertOnce(True)

            # Tocar la notificación abre la app en la pantalla de navegación
            try:
                Intent = autoclass("android.content.Intent")
                PendingIntent = autoclass("android.app.PendingIntent")
                intent = Intent(activity, activity.getClass())
                intent.setAction("OPEN_NAVEGACION")
                intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP)
                flags = PendingIntent.FLAG_UPDATE_CURRENT | 0x02000000  # FLAG_IMMUTABLE
                pi = PendingIntent.getActivity(activity, 0, intent, flags)
                builder.setContentIntent(pi)
                builder.setAutoCancel(False)
            except Exception:
                pass

            notif = builder.build()
            notif_service.notify(cls.NOTIF_ID, notif)
        except Exception as e:
            # Fallback con icono del sistema
            try:
                R = autoclass("android.R$drawable")
                builder.setSmallIcon(R.ic_dialog_info)
                notif = builder.build()
                notif_service.notify(cls.NOTIF_ID, notif)
            except Exception:
                pass

    @classmethod
    def cancelar(cls):
        """Elimina la notificación al finalizar la ruta."""
        if platform != "android":
            return
        try:
            activity = PythonActivity.mActivity
            if activity:
                notif_service = activity.getSystemService(Context.NOTIFICATION_SERVICE)
                if notif_service:
                    notif_service.cancel(cls.NOTIF_ID)
        except Exception:
            pass

    @classmethod
    def activar_modo_segundo_plano(cls):
        """Adquiere un WakeLock para que el CPU, GPS y Bluetooth sigan funcionando con pantalla bloqueada."""
        if platform != "android":
            return
        try:
            if cls._wake_lock is None or not cls._wake_lock.isHeld():
                activity = PythonActivity.mActivity
                if activity:
                    pm = activity.getSystemService(Context.POWER_SERVICE)
                    cls._wake_lock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Centinela:NavegacionWakeLock")
                    cls._wake_lock.acquire()
        except Exception:
            pass

    @classmethod
    def desactivar_modo_segundo_plano(cls):
        """Libera el WakeLock al detener el GPS o navegación."""
        if platform != "android":
            return
        try:
            if cls._wake_lock is not None and cls._wake_lock.isHeld():
                cls._wake_lock.release()
                cls._wake_lock = None
        except Exception:
            pass
