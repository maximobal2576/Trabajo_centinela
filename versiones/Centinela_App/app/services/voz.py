from kivy.utils import platform


class VozService:
    """Servicio integral de voz para Centinela:
    1. Text-To-Speech (TTS) nativo de Android: El teléfono le habla al usuario ciego.
    2. Speech-To-Text (STT): Reconocimiento de comandos por micrófono del celular o del bastón.
    """

    CODIGO_SOLICITUD = 4201
    _tts_instance = None
    _tts_listo = False

    def __init__(self):
        self._callback = None
        self._inicializar_tts()

    # =========================================================================
    # SÍNTESIS DE VOZ (TEXT-TO-SPEECH) — EL TELÉFONO LE HABLA AL CIEGO
    # =========================================================================

    @classmethod
    def _inicializar_tts(cls):
        """Inicializa el motor nativo de TextToSpeech de Android de forma asíncrona."""
        if cls._tts_instance is not None or platform != "android":
            return

        try:
            from jnius import autoclass, PythonJavaClass, java_method

            TextToSpeech = autoclass("android.speech.tts.TextToSpeech")
            PythonActivity = autoclass("org.kivy.android.PythonActivity")
            Locale = autoclass("java.util.Locale")

            class TTSInitListener(PythonJavaClass):
                __javainterfaces__ = ["android/speech/tts/TextToSpeech$OnInitListener"]
                __javacontext__ = "app"

                @java_method("(I)V")
                def onInit(self, status):
                    if status == TextToSpeech.SUCCESS:
                        VozService._tts_listo = True
                        loc_es = Locale("es", "ES")
                        if VozService._tts_instance.isLanguageAvailable(loc_es) >= 0:
                            VozService._tts_instance.setLanguage(loc_es)
                        else:
                            loc_ar = Locale("es", "AR")
                            VozService._tts_instance.setLanguage(loc_ar)
                        VozService._tts_instance.setSpeechRate(1.05)

            listener = TTSInitListener()
            cls._tts_instance = TextToSpeech(PythonActivity.mActivity, listener)
        except Exception as e:
            print(f"[VozService] Error al inicializar TTS Android: {e}")

    @classmethod
    def hablar(cls, texto: str, interrumpir: bool = True):
        """Pronuncia un mensaje en voz alta en español.
        Esencial para la autonomía total del usuario ciego.
        """
        if not texto:
            return

        print(f"[Voz] {texto}")

        if platform == "android":
            if cls._tts_instance is None:
                cls._inicializar_tts()

            try:
                from jnius import autoclass
                TextToSpeech = autoclass("android.speech.tts.TextToSpeech")
                modo_cola = TextToSpeech.QUEUE_FLUSH if interrumpir else TextToSpeech.QUEUE_ADD
                cls._tts_instance.speak(texto, modo_cola, None, "centinela_tts")
            except Exception as e:
                print(f"[VozService] Error en speak Android: {e}")
        else:
            # Fallback en Windows / Desktop para pruebas
            try:
                import pyttsx3
                engine = pyttsx3.init()
                engine.say(texto)
                engine.runAndWait()
            except Exception:
                pass

    # =========================================================================
    # RECONOCIMIENTO DE VOZ DEL MICRÓFONO DEL CELULAR
    # =========================================================================

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
            intent.putExtra(RecognizerIntent.EXTRA_PROMPT, "Decí: estado, dónde estoy, o iniciar navegación")

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

    # =========================================================================
    # RECONOCIMIENTO DE AUDIO REMOTO (DEL MICRÓFONO DEL BASTÓN)
    # =========================================================================

    def procesar_audio_pcm(self, pcm_data: bytes, callback):
        if not pcm_data:
            if callback:
                from kivy.clock import Clock
                Clock.schedule_once(lambda dt: callback(None, "Audio vacío"), 0)
            return

        import threading
        hilo = threading.Thread(
            target=self._transcribir_audio_remoto,
            args=(pcm_data, callback),
            daemon=True,
        )
        hilo.start()

    def _transcribir_audio_remoto(self, pcm_data: bytes, callback):
        import struct
        import json
        import urllib.request
        from kivy.clock import Clock

        sample_rate = 16000
        channels = 1
        bits_per_sample = 16
        byte_rate = sample_rate * channels * (bits_per_sample // 8)
        block_align = channels * (bits_per_sample // 8)
        data_size = len(pcm_data)
        chunk_size = 36 + data_size

        wav_header = struct.pack(
            "<4sI4s4sIHHIIHH4sI",
            b"RIFF",
            chunk_size,
            b"WAVE",
            b"fmt ",
            16,
            1,
            channels,
            sample_rate,
            byte_rate,
            block_align,
            bits_per_sample,
            b"data",
            data_size,
        )
        wav_bytes = wav_header + pcm_data

        texto_reconocido = None
        error_msg = None

        try:
            url = "https://www.google.com/speech-api/v2/recognize?client=chromium&lang=es-419"
            req = urllib.request.Request(
                url,
                data=wav_bytes,
                headers={"Content-Type": "audio/x-wav; rate=16000"},
            )
            with urllib.request.urlopen(req, timeout=7) as respuesta:
                cuerpo = respuesta.read().decode("utf-8", errors="ignore")
                for linea in cuerpo.splitlines():
                    linea = linea.strip()
                    if not linea:
                        continue
                    try:
                        data = json.loads(linea)
                        resultados = data.get("result", [])
                        if resultados and "alternative" in resultados[0]:
                            alternativas = resultados[0]["alternative"]
                            if alternativas and "transcript" in alternativas[0]:
                                texto_reconocido = alternativas[0]["transcript"].strip()
                                break
                    except Exception:
                        pass
        except Exception as e:
            error_msg = f"Error al procesar voz remota: {e}"

        if not texto_reconocido and not error_msg:
            error_msg = "No se reconoció ninguna orden de voz."

        if callback:
            Clock.schedule_once(lambda dt: callback(texto_reconocido, error_msg), 0)
