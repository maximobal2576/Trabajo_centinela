# -*- coding: utf-8 -*-
"""
services/geocodificador.py - Servicios de consulta a APIs de mapas y geolocalizacion para Centinela.

Responsabilidades:
1. Explorar el entorno inmediato del usuario (Reverse Geocoding con OpenStreetMap / Nominatim)
   para anunciar por voz la calle, altura, barrio o comercio donde se encuentra parado.
2. Normalizar y corregir errores foneticos en comandos de voz o texto hacia destinos urbanos
   (ej. "farmasia" -> "farmacia", "ypf" -> "estacion ypf").
3. Buscar lugares candidatos usando Photon (Komoot) y Nominatim combinando similitud textual
   y cercania geografica en metros al usuario.
"""

import difflib
import json
import ssl
import threading
import unicodedata
import urllib.parse
import urllib.request

from services.calculos_geo import distancia_metros

# Agente de usuario para politicas de uso de OpenStreetMap
CABECERAS_HTTP = {"User-Agent": "CentinelaApp/1.0 (centinela.app@gmail.com)"}


def remover_tildes(texto):
    """Elimina acentos y tildes para comparacion difusa de texto sin distinguir acentos."""
    if not texto:
        return ""
    return "".join(
        c for c in unicodedata.normalize("NFD", texto)
        if unicodedata.category(c) != "Mn"
    )


def normalizar_consulta_destino(texto):
    """
    Genera variantes inteligentes de busqueda:
    - Remueve frases de inicio coloquiales ("ir a", "cómo llegar a", "dónde queda").
    - Corrige errores frecuentes de dictado por voz ("farmasia", "kiosko", "mcdonals").
    - Agrega variantes sin tildes e inversas.
    """
    texto_limpio = texto.strip()
    prefijos = (
        "ir a la", "ir al", "ir a", "a la", "al", "a el", "la", "el", "los", "las",
        "cómo llegar a", "como llegar a", "como ir a", "cómo ir a", "llegar a",
        "navegar a", "navegar al", "ir hacia", "buscar", "dónde queda", "donde queda",
    )
    for prefijo in prefijos:
        if texto_limpio.lower().startswith(f"{prefijo} "):
            texto_limpio = texto_limpio[len(prefijo):].strip()
            break

    sin_tildes = remover_tildes(texto_limpio)
    variantes = [texto_limpio]
    if sin_tildes.lower() != texto_limpio.lower():
        variantes.append(sin_tildes)

    # Diccionario de correccion fonetica y sinonimos urbanos habituales
    errores_y_sinonimos = {
        "farmasia": "farmacia",
        "farmasias": "farmacia",
        "farmacias": "farmacia",
        "estasion": "estacion",
        "super": "supermercado",
        "supermercados": "supermercado",
        "hipemercado": "hipermercado",
        "hiper": "hipermercado",
        "kiosko": "kiosco",
        "kioskos": "kiosco",
        "quiosco": "kiosco",
        "quioscos": "kiosco",
        "panaderias": "panaderia",
        "panaderías": "panaderia",
        "heladerias": "heladeria",
        "heladerías": "heladeria",
        "carnicerias": "carniceria",
        "carnicerías": "carniceria",
        "ferreterias": "ferreteria",
        "ferreterías": "ferreteria",
        "librerias": "libreria",
        "librerías": "libreria",
        "pinturerias": "pintureria",
        "pinturerías": "pintureria",
        "zapaterias": "zapateria",
        "zapaterías": "zapateria",
        "plazas": "plaza",
        "parques": "parque",
        "hospitales": "hospital",
        "sanatorios": "sanatorio",
        "clinicas": "clinica",
        "clínicas": "clinica",
        "veterinarias": "veterinaria",
        "ypf": "estacion ypf",
        "shell": "estacion shell",
        "axion": "estacion axion",
        "nafta": "estacion de servicio",
        "gasolinera": "estacion de servicio",
        "cajero": "cajero automatico",
        "banelco": "cajero banelco",
        "link": "cajero red link",
        "parada": "parada de colectivo",
        "colectivo": "parada de colectivo",
        "mcdonals": "mcdonald's",
        "mcdonal": "mcdonald's",
    }

    palabras = texto_limpio.split()
    palabras_corr = [errores_y_sinonimos.get(remover_tildes(p).lower(), p) for p in palabras]
    texto_corr = " ".join(palabras_corr)
    if texto_corr.lower() != texto_limpio.lower() and texto_corr not in variantes:
        variantes.append(texto_corr)

    if len(palabras_corr) == 2:
        invertido = f"{palabras_corr[1]} {palabras_corr[0]}"
        if invertido not in variantes:
            variantes.append(invertido)

    for p in palabras_corr:
        if len(p) >= 4 and p not in variantes and p.lower() not in ("calle", "avenida", "diagonal"):
            variantes.append(p)

    return variantes


def calcular_similitud_texto(busqueda, texto_candidato):
    """
    Calcula un porcentaje de coincidencia entre dos textos (de 0.0 a 1.0)
    usando comparacion por tokens (Jaccard) y distancia difusa (SequenceMatcher).
    """
    b = remover_tildes(busqueda).lower().strip()
    c = remover_tildes(texto_candidato).lower().strip()
    if not b or not c:
        return 0.0
    if b == c:
        return 1.0
    if b in c or c in b:
        return 0.92

    ratio = difflib.SequenceMatcher(None, b, c).ratio()
    b_words = set(b.split())
    c_words = set(c.split())
    if b_words and c_words:
        inter = len(b_words & c_words)
        union = len(b_words | c_words)
        jaccard = inter / union if union > 0 else 0
        if inter == len(b_words):
            return max(0.90, ratio)
        return max(ratio, jaccard * 0.85)

    return ratio


def evaluar_mejor_candidato(destino_buscado, lista_candidatos, lat_usr, lon_usr):
    """
    Clasifica los candidatos combinando 60% similitud de texto y 40% proximidad fisica al usuario.
    Permite encontrar el lugar correcto aunque haya pequeños errores en el nombre o direccion.
    """
    if not lista_candidatos:
        return None

    mejor_candidato = None
    mejor_puntaje = -999999.0

    for cand in lista_candidatos:
        lat = cand.get("lat")
        lon = cand.get("lon")
        if lat is None or lon is None:
            continue

        dist_m = distancia_metros(
            {"latitud": lat_usr, "longitud": lon_usr},
            {"latitud": lat, "longitud": lon}
        )

        nombre_corto = cand.get("name") or cand.get("nombre") or ""
        nombre_largo = cand.get("display_name") or ""
        categoria = cand.get("category") or cand.get("type") or ""

        sim_c = calcular_similitud_texto(destino_buscado, nombre_corto)
        sim_l = calcular_similitud_texto(destino_buscado, nombre_largo)
        sim_cat = calcular_similitud_texto(destino_buscado, categoria) * 0.75
        similitud = max(sim_c, sim_l, sim_cat)

        if dist_m <= 300:
            puntos_dist = 100.0
        elif dist_m <= 1000:
            puntos_dist = 90.0 - ((dist_m - 300) / 700) * 15.0
        elif dist_m <= 5000:
            puntos_dist = 75.0 - ((dist_m - 1000) / 4000) * 25.0
        elif dist_m <= 25000:
            puntos_dist = 50.0 - ((dist_m - 25000) / 20000) * 30.0
        elif dist_m <= 80000:
            puntos_dist = 20.0 - ((dist_m - 25000) / 55000) * 20.0
        else:
            puntos_dist = -50.0 - min(100.0, (dist_m - 80000) / 10000)

        puntaje_total = (similitud * 100.0 * 0.60) + (puntos_dist * 0.40)
        if similitud >= 0.88:
            puntaje_total += 25.0

        if puntaje_total > mejor_puntaje:
            mejor_puntaje = puntaje_total
            etiqueta = nombre_corto or nombre_largo.split(",")[0]
            dist_texto = f"a {dist_m:.0f} m" if dist_m < 1000 else f"a {dist_m / 1000:.1f} km"

            partes = [p.strip() for p in nombre_largo.split(",") if p.strip()]
            calle_barrio = ""
            if len(partes) > 1 and partes[1].lower() != etiqueta.lower():
                calle_barrio = f", {partes[1]}"

            mejor_candidato = {
                "lat": lat,
                "lon": lon,
                "nombre": f"{etiqueta}{calle_barrio} ({dist_texto})",
                "distancia_metros": dist_m,
                "similitud": similitud,
            }

    return mejor_candidato


def reverse_geocoding_actual(lat, lon, callback):
    """
    Modo Exploracion: Obtiene mediante reverse geocoding la calle,
    altura, barrio o comercio donde se encuentra el usuario.
    Ejecuta el resultado en segundo plano llamando a callback(texto_resultado, error).
    """
    def _tarea():
        try:
            ctx = ssl._create_unverified_context()
            url = (
                f"https://nominatim.openstreetmap.org/reverse"
                f"?lat={lat}&lon={lon}&format=json&zoom=18&addressdetails=1&accept-language=es"
            )
            req = urllib.request.Request(url, headers=CABECERAS_HTTP)
            with urllib.request.urlopen(req, context=ctx, timeout=8) as resp:
                data = json.loads(resp.read().decode("utf-8"))

            address = data.get("address", {})

            lugar = (
                address.get("amenity")
                or address.get("shop")
                or address.get("tourism")
                or address.get("leisure")
                or address.get("building")
                or ""
            )
            calle = (
                address.get("road")
                or address.get("pedestrian")
                or address.get("path")
                or address.get("footway")
                or ""
            )
            numero = address.get("house_number", "")
            barrio = (
                address.get("suburb")
                or address.get("neighbourhood")
                or address.get("quarter")
                or ""
            )
            ciudad = (
                address.get("city")
                or address.get("town")
                or address.get("village")
                or address.get("county")
                or ""
            )

            partes = []
            if lugar:
                partes.append(lugar)
            if calle:
                partes.append(f"{calle} {numero}".strip())
            if barrio:
                partes.append(barrio)
            if ciudad and ciudad not in " ".join(partes):
                partes.append(ciudad)

            if partes:
                resultado = ", ".join(p for p in partes if p)
            else:
                resultado = data.get("display_name", "Ubicación detectada").split(",")[0]

            callback(f"Estás en: {resultado}", None)
        except Exception as e:
            callback(None, f"No se pudo obtener la dirección: {e}")

    threading.Thread(target=_tarea, daemon=True).start()