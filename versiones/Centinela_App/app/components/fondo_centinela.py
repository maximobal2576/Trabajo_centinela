from kivy.graphics import Color, Rectangle
from kivy.graphics.texture import Texture
from kivy.core.image import Image as CoreImage
from kivy.uix.widget import Widget
import random


class FondoCentinela(Widget):

    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.size_hint = (1, 1)

        # =========================
        # FONDO
        # =========================

        self.texture_degradado = self.crear_degradado()

        # =========================
        # LOGO
        # =========================

        logo_texture = CoreImage(
            "assets/images/logo_centinela3.png"
        ).texture

        with self.canvas:

            # Fondo principal
            self.fondo = Rectangle(
                texture=self.texture_degradado,
                pos=self.pos,
                size=self.size
            )

            # =========================
            # TEXTURA DE TELA
            # =========================

            self.texture_tela = self.crear_textura_tela()

            Color(
                1,
                1,
                1,
                0.05
            )

            self.tela = Rectangle(
                texture=self.texture_tela,
                pos=self.pos,
                size=self.size
            )

            # =========================
            # LOGO
            # =========================

            Color(
                1,
                1,
                1,
                0.16
            )

            self.logo_fondo = Rectangle(
                texture=logo_texture,
                pos=self.pos,
                size=(300, 300)
            )

        self.bind(
            pos=self.actualizar_fondo,
            size=self.actualizar_fondo
        )

        self.actualizar_fondo()

    # ==================================================
    # DEGRADADO CREMA
    # ==================================================

    def crear_degradado(self):

        ancho = 2
        alto = 512

        textura = Texture.create(
            size=(ancho, alto),
            colorfmt="rgba"
        )

        pixels = bytearray()

        oscuro = (55, 43, 32)
        medio = (105, 83, 60)
        claro = (135, 108, 78)

        for y in range(alto):

            t = y / (alto - 1)

            if t < 0.5:

                p = t / 0.5

                r = int(
                    oscuro[0] +
                    (medio[0] - oscuro[0]) * p
                )

                g = int(
                    oscuro[1] +
                    (medio[1] - oscuro[1]) * p
                )

                b = int(
                    oscuro[2] +
                    (medio[2] - oscuro[2]) * p
                )

            else:

                p = (t - 0.5) / 0.5

                r = int(
                    medio[0] +
                    (claro[0] - medio[0]) * p
                )

                g = int(
                    medio[1] +
                    (claro[1] - medio[1]) * p
                )

                b = int(
                    medio[2] +
                    (claro[2] - medio[2]) * p
                )

            pixels.extend([r, g, b, 255])
            pixels.extend([r, g, b, 255])

        textura.blit_buffer(
            pixels,
            colorfmt="rgba",
            bufferfmt="ubyte"
        )

        textura.wrap = "clamp_to_edge"

        return textura

    # ==================================================
    # TEXTURA DE TELA
    # ==================================================

    def crear_textura_tela(self):

        ancho = 256
        alto = 256

        textura = Texture.create(
            size=(ancho, alto),
            colorfmt="rgba"
        )

        pixels = bytearray()

        random.seed(10)

        for y in range(alto):

            for x in range(ancho):

                # Variación muy pequeña
                variacion = random.randint(
                    -12,
                    12
                )

                # Fibra horizontal
                fibra_horizontal = (
                    4
                    if y % 5 == 0
                    else 0
                )

                # Fibra vertical muy tenue
                fibra_vertical = (
                    3
                    if x % 7 == 0
                    else 0
                )

                valor = (
                    128
                    + variacion
                    + fibra_horizontal
                    + fibra_vertical
                )

                valor = max(
                    0,
                    min(255, valor)
                )

                pixels.extend([
                    valor,
                    valor,
                    valor,
                    255
                ])

        textura.blit_buffer(
            pixels,
            colorfmt="rgba",
            bufferfmt="ubyte"
        )

        textura.wrap = "repeat"

        return textura

    # ==================================================
    # ACTUALIZAR
    # ==================================================

    def actualizar_fondo(self, *args):

        self.fondo.pos = self.pos
        self.fondo.size = self.size

        # Textura de tela
        self.tela.pos = self.pos
        self.tela.size = self.size

        # Logo
        textura = self.logo_fondo.texture

        if textura is None:
            return

        ancho = self.width * 0.70

        if ancho <= 0:
            return

        proporcion = (
            textura.height /
            textura.width
        )

        alto = ancho * proporcion

        self.logo_fondo.size = (
            ancho,
            alto
        )

        # Centrado
        self.logo_fondo.pos = (
            self.center_x - ancho / 2,
            self.center_y - alto / 2
        )