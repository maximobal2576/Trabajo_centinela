# -*- coding: utf-8 -*-
"""
services/calculos_geo.py - Utilidades matematicas y geograficas de Centinela.

Provee funciones puras para:
1. Calculo de distancias geodesicas entre coordenadas (Formula de Haversine).
2. Determinacion de rumbos en grados y puntos cardinales (Norte, Sur, Este, Oeste).
3. Interpolacion de waypoints cada ~8 metros para guia continua a pie.
4. Identificacion de la esquina o proximo giro en un trayecto peatonal.
"""

import math

# Distancia estandar entre puntos interpolados para el baston (aprox. 8 metros)
DISTANCIA_MINIMA_METROS = 8

# Angulo minimo de cambio de direccion para considerar una esquina (55 grados)
ANGULO_GIRO_ESQUINA = 55


def distancia_metros(origen, destino):
    """
    Calcula la distancia en metros entre dos coordenadas geograficas
    usando la formula del semiverseno (Haversine).
    Tanto 'origen' como 'destino' deben ser diccionarios con claves 'latitud' y 'longitud'.
    """
    radio_tierra = 6_371_000  # Radio medio de la Tierra en metros
    lat1, lon1 = math.radians(origen["latitud"]), math.radians(origen["longitud"])
    lat2, lon2 = math.radians(destino["latitud"]), math.radians(destino["longitud"])

    dlat = lat2 - lat1
    dlon = lon2 - lon1

    a = math.sin(dlat / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2) ** 2
    return 2 * radio_tierra * math.asin(math.sqrt(a))


def rumbo_grados(origen, destino):
    """
    Calcula el angulo azimutal de navegacion en grados (0 a 360) desde el punto de origen al destino.
    0° = Norte, 90° = Este, 180° = Sur, 270° = Oeste.
    """
    lat1, lon1 = math.radians(origen["latitud"]), math.radians(origen["longitud"])
    lat2, lon2 = math.radians(destino["latitud"]), math.radians(destino["longitud"])

    dlon = lon2 - lon1
    x = math.sin(dlon) * math.cos(lat2)
    y = math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(dlon)

    return math.degrees(math.atan2(x, y)) % 360


def rumbo_cardinal_desde_grados(angulo):
    """Convierte un angulo azimutal en grados al punto cardinal equivalente (8 rumbos)."""
    rumbos = ("norte", "noreste", "este", "sudeste", "sur", "sudoeste", "oeste", "noroeste")
    return rumbos[int((angulo + 22.5) // 45) % len(rumbos)]


def rumbo_cuadra_desde_grados(angulo):
    """
    Convierte un angulo a los 4 rumbos principales ortogonales (cuadras de ciudad: N, E, S, O).
    Es ideal para dar instrucciones auditivas simples al usuario invidente.
    """
    rumbos = ("norte", "este", "sur", "oeste")
    return rumbos[int((angulo + 45) // 90) % len(rumbos)]


def rumbo_cardinal(origen, destino):
    """Devuelve el nombre del rumbo cardinal directo entre dos puntos GPS."""
    return rumbo_cardinal_desde_grados(rumbo_grados(origen, destino))


def interpolar_puntos_cada_8m(puntos_base):
    """
    Rellena los espacios vacios entre coordenadas de la ruta colocando puntos intermedios
    cada ~8 metros. Esto asegura que el baston y la app puedan verificar el avance
    en tiempo real sin saltos bruscos.
    """
    if not puntos_base:
        return []
    puntos_densos = [puntos_base[0]]
    for i in range(len(puntos_base) - 1):
        p1 = puntos_base[i]
        p2 = puntos_base[i + 1]
        dist = distancia_metros(p1, p2)
        if dist > DISTANCIA_MINIMA_METROS:
            cant_segmentos = int(dist // DISTANCIA_MINIMA_METROS)
            for seg in range(1, cant_segmentos + 1):
                frac = seg / (cant_segmentos + 1)
                lat_int = p1["latitud"] + frac * (p2["latitud"] - p1["latitud"])
                lon_int = p1["longitud"] + frac * (p2["longitud"] - p1["longitud"])
                puntos_densos.append({"latitud": lat_int, "longitud": lon_int})
        puntos_densos.append(p2)
    return puntos_densos


def rumbo_del_tramo(puntos, indice):
    """Obtiene el rumbo del tramo actual de la lista de puntos."""
    if indice >= len(puntos) - 1:
        return rumbo_grados(puntos[-2], puntos[-1])
    return rumbo_grados(puntos[indice], puntos[indice + 1])


def proxima_esquina(puntos, inicio):
    """
    Analiza los puntos subsiguientes para detectar la proxima esquina
    (cambio de direccion mayor o igual a 55 grados) y el rumbo que tendra despues del giro.
    """
    ultimo = len(puntos) - 1
    if inicio >= ultimo:
        return ultimo, None
    rumbo_base = rumbo_del_tramo(puntos, inicio)
    for indice in range(inicio + 1, ultimo):
        rumbo_siguiente = rumbo_del_tramo(puntos, indice)
        diferencia = abs((rumbo_siguiente - rumbo_base + 180) % 360 - 180)
        if diferencia >= ANGULO_GIRO_ESQUINA:
            return indice, rumbo_cuadra_desde_grados(rumbo_siguiente)
    return ultimo, None


def punto_mas_cercano(puntos, ubicacion):
    """Encuentra el punto de una lista que se encuentre a menor distancia de una coordenada."""
    if not puntos or not ubicacion:
        return None, None
    indice, punto = min(enumerate(puntos), key=lambda item: distancia_metros(ubicacion, item[1]))
    return indice, punto