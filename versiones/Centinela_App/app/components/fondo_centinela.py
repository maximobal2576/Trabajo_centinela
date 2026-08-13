from kivy.graphics import Color, Rectangle
from kivy.graphics.texture import Texture
from kivy.core.image import Image as CoreImage
from kivy.uix.widget import Widget


class FondoCentinela(Widget):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.size_hint = (1, 1)
        self.pos_hint = {"x": 0, "y": 0}

        # Crear fondo
        self.texture_degradado = self.crear_degradado()

        # Cargar logo
        logo_texture = CoreImage(
            "assets/images/logo_centinela.png"
        ).texture

        with self.canvas:

            # Fondo
            self.color_fondo = Color(
                1, 1, 1, 1
            )

            self.fondo = Rectangle(
                texture=self.texture_degradado,
                pos=self.pos,
                size=self.size
            )

            # Logo como marca de agua
            self.color_logo = Color(
                0.42, 0.12, 0.18, 0.08
            )

            self.logo_fondo = Rectangle(
                texture=logo_texture,
                pos=self.pos,
                size=(400, 400)
            )

        self.bind(
            pos=self.actualizar_fondo,
            size=self.actualizar_fondo
        )

    def crear_degradado(self):

        ancho = 2
        alto = 512

        textura = Texture.create(
            size=(ancho, alto),
            colorfmt="rgba"
        )

        pixels = bytearray()

        for y in range(alto):

            t = y / (alto - 1)

            # -------------------------
            # COLORES DEL FONDO
            # -------------------------

            # Parte superior:
            # Sepia oscuro
            color_inicio = (
                111,
                90,
                69
            )

            # Centro:
            # Crema envejecido
            color_medio = (
                155,
                132,
                98
            )

            # Parte inferior:
            # Bordó profundo
            color_final = (
                84,
                23,
                34
            )

            # -------------------------
            # PRIMERA MITAD
            # Sepia → crema
            # -------------------------

            if t < 0.55:

                progreso = t / 0.55

                r = int(
                    color_inicio[0]
                    + (
                        color_medio[0]
                        - color_inicio[0]
                    ) * progreso
                )

                g = int(
                    color_inicio[1]
                    + (
                        color_medio[1]
                        - color_inicio[1]
                    ) * progreso
                )

                b = int(
                    color_inicio[2]
                    + (
                        color_medio[2]
                        - color_inicio[2]
                    ) * progreso
                )

            # -------------------------
            # SEGUNDA MITAD
            # Crema → bordó
            # -------------------------

            else:

                progreso = (
                    (t - 0.55) / 0.45
                )

                r = int(
                    color_medio[0]
                    + (
                        color_final[0]
                        - color_medio[0]
                    ) * progreso
                )

                g = int(
                    color_medio[1]
                    + (
                        color_final[1]
                        - color_medio[1]
                    ) * progreso
                )

                b = int(
                    color_medio[2]
                    + (
                        color_final[2]
                        - color_medio[2]
                    ) * progreso
                )

            for _ in range(ancho):
                pixels.extend(
                    [r, g, b, 255]
                )

        textura.blit_buffer(
            pixels,
            colorfmt="rgba",
            bufferfmt="ubyte"
        )

        textura.wrap = "clamp_to_edge"

        return textura

    def actualizar_fondo(self, *args):

        self.fondo.pos = self.pos
        self.fondo.size = self.size

        tamaño = min(
            self.width,
            self.height
        ) * 0.75

        self.logo_fondo.size = (
            tamaño,
            tamaño
        )

        self.logo_fondo.pos = (
            self.center_x - tamaño / 2,
            self.center_y - tamaño / 2
        )