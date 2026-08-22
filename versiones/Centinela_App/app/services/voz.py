from kivy.utils import platform


class VozService:
    """Reconocimiento de voz nativo de Android para comandos cortos."""

    CODIGO_SOLICITUD = 4201

    def __init__(self):
        self._callback = None

    def escuchar(self, callback):
        self._callback = callback
        if platform != "android":
            callback(None, "El micrófono se usa desde la aplicación Android.")
            return

        from android.permissions import Permission, check_permission, request_permissions
        if not check_permission(Permission.RECORD_AUDIO):
            request_permissions([Permission.RECORD_AUDIO], self._al_resolver_permiso)
            callback(None, "Autorizá el micrófono y tocá el botón nuevamente.")
            return
        self._abrir_reconocedor()

    def _al_resolver_permiso(self, _permisos, _resultados):
        pass

    def _abrir_reconocedor(self):
        try:
            from jnius import autoclass
            from android import activity
            Intent = autoclass("android.content.Intent")
            RecognizerIntent = autoclass("android.speech.RecognizerIntent")
            PythonActivity = autoclass("org.kivy.android.PythonActivity")

            intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH)
            intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
            intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, "es-AR")
            intent.putExtra(RecognizerIntent.EXTRA_PROMPT, "Decí: iniciar ubicación, guardar ruta o retomar ruta")

            activity.bind(on_activity_result=self._al_resultado)
            PythonActivity.mActivity.startActivityForResult(intent, self.CODIGO_SOLICITUD)
        except Exception as error:
            self._finalizar(None, f"No se pudo abrir el micrófono: {error}")

    def _al_resultado(self, request_code, result_code, intent):
        if request_code != self.CODIGO_SOLICITUD:
            return
        try:
            from jnius import autoclass
            Activity = autoclass("android.app.Activity")
            if result_code != Activity.RESULT_OK or intent is None:
                self._finalizar(None, "No se reconoció ningún comando.")
                return
            RecognizerIntent = autoclass("android.speech.RecognizerIntent")
            resultados = intent.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS)
            texto = resultados.get(0) if resultados and resultados.size() else None
            self._finalizar(texto, None if texto else "No se reconoció ningún comando.")
        except Exception as error:
            self._finalizar(None, f"Error al reconocer voz: {error}")

    def _finalizar(self, texto, error):
        if platform == "android":
            try:
                from android import activity
                activity.unbind(on_activity_result=self._al_resultado)
            except Exception:
                pass
        callback, self._callback = self._callback, None
        if callback:
            from kivy.clock import Clock
            cb = callback
            Clock.schedule_once(lambda dt: cb(texto, error), 0)
