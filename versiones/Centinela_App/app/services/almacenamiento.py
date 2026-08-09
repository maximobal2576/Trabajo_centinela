import json
from pathlib import Path

from kivymd.app import MDApp


class AlmacenamientoService:
    """Guarda ajustes simples de la aplicación en un archivo local."""

    ARCHIVO = "configuracion_centinela.json"

    @classmethod
    def cargar_configuracion(cls):
        ruta = cls._ruta_configuracion()

        if not ruta.exists():
            return {"contacto_emergencia": ""}

        try:
            with ruta.open("r", encoding="utf-8") as archivo:
                configuracion = json.load(archivo)
        except (OSError, json.JSONDecodeError):
            return {"contacto_emergencia": ""}

        configuracion.setdefault("contacto_emergencia", "")
        return configuracion

    @classmethod
    def guardar_contacto_emergencia(cls, numero):
        configuracion = cls.cargar_configuracion()
        configuracion["contacto_emergencia"] = numero.strip()

        ruta = cls._ruta_configuracion()
        ruta.parent.mkdir(parents=True, exist_ok=True)

        with ruta.open("w", encoding="utf-8") as archivo:
            json.dump(configuracion, archivo, ensure_ascii=False)

    @classmethod
    def _ruta_configuracion(cls):
        app = MDApp.get_running_app()

        if app is not None:
            try:
                return Path(app.user_data_dir) / cls.ARCHIVO
            except OSError:
                pass

        return Path.cwd() / cls.ARCHIVO
