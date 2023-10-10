#include "../Comportamientos_Jugador/jugador.hpp"
#include <iostream>
#include <assert.h>
using namespace std;

/**
 * Dos partes:
 * (a) Actualizar variables de estado y observar cómo ha cambiado el mundo y los sensores.
 * (b) Se determina la acción a realizar (en consecuencia con lo visto).
 */
Action ComportamientoJugador::think(Sensores sensores) {

    // (a) Actualizar información
    if (inicio_juego) {
        // Inicializa los precipicios
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < map_size; j++) {
                mapaResultado[i][j] = 'P';
                mapaResultado[map_size - i - 1][j] = 'P';
                mapaResultado[j][i] = 'P';
                mapaResultado[j][map_size - i - 1] = 'P';
            }
        }
        inicio_juego = false;
    }

    if (sensores.reset) reestablecer();

    if (sensores.nivel == 0) {
        bien_posicionado = true;
        fil = sensores.posF;
        col = sensores.posC;
        brujula = sensores.sentido;
    } else {
        if(!sensores.colision) {
            switch (ultimaAccion) {
                case (actFORWARD):
                    switch (brujula) {
                        case (0):
                            fil--;
                            break; // Norte
                        case (1):
                            col++;
                            break; // Este
                        case (2):
                            fil++;
                            break; // Sur
                        case (3):
                            col--;
                            break; // Oeste
                    }
                    break;
                case (actTURN_R):
                    brujula = (brujula + 1) % 4;
                    girar_dcha = (rand() % 2 == 0);
                    break;
                case (actTURN_L):
                    brujula = (brujula + 3) % 4;
                    girar_dcha = (rand() % 2 == 0);
                    break;
            }
        }
    }

    // Actualiza las variables acorde con la información de los sensores
    if (bien_posicionado)
        actualizaMapaResultado(sensores);
    else {
        if (sensores.terreno[0] == 'G') {
            // Se vuelca la matriz mapaProvisional en mapaResultando teniendo en cuenta los valores de fila, columna y
            // brújula relativos a mapaProvisional
            transforma_matriz(sensores);
            // Actualiza las posiciones en valores correctos
            bien_posicionado = true;
            fil = sensores.posF;
            col = sensores.posC;
            brujula = sensores.sentido;
        } else {
            actualizaMapaProvisional(sensores);
        }
    }

    // Actualiza información necesaria para determinar la acción a realizar
    if (sensores.terreno[0] == 'K' && !tiene_bikini) tiene_bikini = true;
    else if (sensores.terreno[0] == 'D' && !tiene_zapatillas) tiene_zapatillas = true;
    else if (sensores.terreno[0] == 'X' && sensores.bateria < min_bateria_para_recargar) recargar_bateria = true;

    // Si tiene colisión, aborta la acción (la secuencia de acciones no es viable)
    if (sensores.colision) {
        // vacía la cola
        while (!accionCompuesta.empty()) accionCompuesta.pop();
    }

    // Si encuentra alguna de las prioridades primarias, establece la accion a tratar
    if (objetivo < 0) {
        if ((objetivo = en_vision('X', sensores)) >= 0 &&
            sensores.bateria < min_bateria_para_recargar) { // Prioridad 0: Recargar batería (si está baja)
            perseguir(objetivo);
        } else if ((objetivo = en_vision('G', sensores)) >= 0 &&
                   !bien_posicionado) { // Prioridad 1: Localizarse, si ve la casilla G y no está bien posicionado, va hacia ella
            perseguir(objetivo);
        } else if ((objetivo = en_vision('K', sensores)) >= 0 &&
                   !tiene_bikini) { // Prioridad 2: Bikini (si no lo tiene)
            perseguir(objetivo);
        } else if ((objetivo = en_vision('D', sensores)) >= 0 && !tiene_zapatillas) { // Prioridad 3: Zapatillas
            perseguir(objetivo);
        }
    }

    // Si la cola está vacía, no hay ningún objetivo
    if (accionCompuesta.empty()) objetivo = -1;

    // (b) Determinar acción a realizar (COMPLETAR)
    Action accion = actIDLE;
    girar_dcha = rand() % 2;    // Para determinar el sentido del giro

    if (recargar_bateria) {
        if (sensores.bateria < 5000) accion = actIDLE;
        else {
            accion = actTURN_L; // Aleatorio, no hay motivo aparente
            recargar_bateria = false;
        }
    } else if (sensores.superficie[2] == 'a' ||
               sensores.superficie[2] == 'l') { // Si se encuentra con alguno de ellos, espera a que reaccionen
        accion = actIDLE;
    } else if (!accionCompuesta.empty()) { // Si está en mitad de una acción compuesta, la ejecuta
        accion = accionCompuesta.front();
        accionCompuesta.pop();
    } else if ( sensores.bateria<=(min_bateria_para_recargar+500) &&
                ( (sensores.terreno[0]!='A' && sensores.terreno[2]=='A' && !tiene_bikini) ||
                (sensores.terreno[0]!='B' && sensores.terreno[2]=='B' &&  !tiene_zapatillas) ) ){
        if (girar_dcha) accion = actTURN_R;
        else accion = actTURN_L;
    } else if(salida_obstaculo_izq) {
        accion = actTURN_L;
        salida_obstaculo_izq = false;
    } else if (salida_obstaculo_dcha) {
        accion = actTURN_R;
        salida_obstaculo_dcha = false;
    } else if ( (sensores.terreno[1]!='M' && sensores.terreno[1]!='P') && (sensores.terreno[5]=='M' || sensores.terreno[5]=='P')) {
        salida_obstaculo_izq = true;
        accion = actFORWARD;
    } else if ( (sensores.terreno[3]!='M' && sensores.terreno[3]!='P') && (sensores.terreno[7]=='M' || sensores.terreno[7]=='P')) {
        salida_obstaculo_dcha = true;
        accion = actFORWARD;
    } else if(sensores.terreno[2]=='M' || sensores.terreno[2]=='P'  || contador_forward>=max_forward) {
        if (girar_dcha) accion = actTURN_R;
        else accion = actTURN_L;
    } else {
        accion = actFORWARD;
    }

    // Para proporcionar movimientos aleatorios (evita los bucles)
    if(accion == actFORWARD)
        contador_forward++;
    else if (accion==actTURN_R || accion==actTURN_L) {
        contador_forward=0;
        max_forward=rand()%(map_size);
    }

    ultimaAccion = accion;

	return accion;
}

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}

void ComportamientoJugador::init() {
    ultimaAccion = actIDLE;
    bien_posicionado = false;
    girar_dcha = false;


    // Variable relativas en función a la posicion
    // bien_posicionado -> valores reales en mapaResultado
    // !bien_posicionado -> valores relativos en mapaProvisional
    brujula = 0; // norte
    fil = col = map_size;

    // Actualiza las variables max y min coords
    min_coordenadas.first = max_coordenadas.first = fil;
    min_coordenadas.second = max_coordenadas.second = col;

    // Inicializa la memoria provisional (las dimensiones son el doble del mapa actual)
    vector<unsigned char> aux(2*map_size, UNKNOWN);
    for(int i=0; i< 2*map_size; i++)
    mapaProvisional.push_back(aux);

    // Variables extra
    contador_forward=0;
    max_forward = rand()%map_size;
    salida_obstaculo_dcha = false;
    salida_obstaculo_izq = false;

    tiene_bikini = false;
    tiene_zapatillas = false;

    objetivo=-1;
    recargar_bateria = false;
}

int ComportamientoJugador::en_vision(unsigned char obj, Sensores sensores) {
    int pos = -1;
    for (int i=0; i<sensores.terreno.size() && pos<0; ++i)
        if(sensores.terreno.at(i)==obj) pos=i;

    return pos;
}

void ComportamientoJugador::perseguir(int i) {
    // Si hay una accion compuesta en marcha, vacía la cola
    while(!accionCompuesta.empty())
        accionCompuesta.pop();

    // Gira para encarar
    if(lateralidad(i)<0) accionCompuesta.push(actTURN_L);
    else if (lateralidad(i)>0) accionCompuesta.push(actTURN_R);

    for(int j=0; j<abs(lateralidad(i)); ++j)
        accionCompuesta.push(actFORWARD);

    // Deshace el giro
    if(accionCompuesta.front() == actTURN_R) accionCompuesta.push(actTURN_L);
    else if (accionCompuesta.front() == actTURN_L) accionCompuesta.push(actTURN_R);

    // Avanza hasta llegar al objetivo
    for (int j=0; j<profundidad(i); ++j)
        accionCompuesta.push(actFORWARD);
}

int ComportamientoJugador::profundidad(int i) {
    assert(0<=i && 0<16);

    int prof;
    if(i==0) prof = 0;
    else if(1<=i && i<=3) prof = 1;
    else if(4<=i && i<=8) prof = 2;
    else if(9<=i && i<=15) prof = 3;

    return prof;
}

int ComportamientoJugador::lateralidad(int i) {
    assert(0<=i && 0<16);

    int lat;
    if(i==0) lat = 0;
    else if(1<=i && i<=3) lat = i-2;
    else if(4<=i && i<=8) lat = i-6;
    else if(9<=i && i<=15) lat = i-12;

    return lat;
}

void ComportamientoJugador::actualizaMapaResultado(Sensores sensores) {
    // assert(bien_posicionado);
    for(int i=0; i<sensores.terreno.size(); i++) {
        int fila_real, columna_real;
        switch(brujula) {
            case(0): fila_real=fil-profundidad(i); columna_real=col+lateralidad(i); break;  // Norte
            case(1): fila_real=fil+lateralidad(i); columna_real=col+profundidad(i); break;  // Este
            case(2): fila_real=fil+profundidad(i); columna_real=col-lateralidad(i); break;  // Sur
            case(3): fila_real=fil-lateralidad(i); columna_real=col-profundidad(i); break;  // Oeste
        }

        // Comprueba el caso en el que la visión vaya más alla del borde del mapa
        if((0<=fila_real && fila_real < map_size) && (0 <= columna_real && columna_real < map_size) ) {
            if(mapaResultado[fila_real][columna_real]==UNKNOWN)
                mapaResultado[fila_real][columna_real] = sensores.terreno[i];
        }
    }

}

void ComportamientoJugador::actualizaMapaProvisional(Sensores sensores) {
    // assert(!bien_posicionado);

    for(int i=0; i<sensores.terreno.size(); i++) {
        int fila_prov, columna_prov;
        switch(brujula) {
            case(0): fila_prov = fil - profundidad(i); columna_prov= col + lateralidad(i); break;  // Norte
            case(1): fila_prov = fil + lateralidad(i); columna_prov= col + profundidad(i); break;  // Este
            case(2): fila_prov = fil + profundidad(i); columna_prov= col - lateralidad(i); break;  // Sur
            case(3): fila_prov = fil - lateralidad(i); columna_prov= col - profundidad(i); break;  // Oeste
        }

        // Comprueba el caso en el que la visión vaya más alla del borde del mapa
        if( (0<= fila_prov && fila_prov<mapaProvisional.size()) && (0<=columna_prov && columna_prov<mapaProvisional[fila_prov].size()) ) {
            mapaProvisional[fila_prov][columna_prov] = sensores.terreno[i];

            // Actualiza min_coordenadas y max_coordenadas en el caso que sea necesario
            if(fila_prov<min_coordenadas.first) min_coordenadas.first = fila_prov;
            else if (fila_prov>max_coordenadas.first) max_coordenadas.first = fila_prov;

            if (columna_prov<min_coordenadas.second) min_coordenadas.second = columna_prov;
            else if (columna_prov>max_coordenadas.second) max_coordenadas.second = columna_prov;
        }
    }
}

int ComportamientoJugador::rotacion(int orientacion_real) {
    int i = (90 * (orientacion_real - brujula)) % 360;
    if(i < 0) i+=360;
    return i;
}

void ComportamientoJugador::submatriz() {

    // Redimensiona la matriz mapaProvisional y actualiza la posicion en ella
    int total_filas = max_coordenadas.first-min_coordenadas.first+1;
    int total_cols = max_coordenadas.second-min_coordenadas.second+1;
    vector<vector<unsigned char>> sub(total_filas);

    for(int i=0; i<total_filas; ++i) {
        for(int j=0; j<total_cols; ++j) {
            int f = min_coordenadas.first+i;    // Fila en mapaProvisional
            int c = min_coordenadas.second+j;   // Columna en mapaProvisional
            sub[i].push_back(mapaProvisional[f][c]);
            if(fil==f && col==c) {  // Actualiza la nueva posición relativa en un mapa más pequeño
                fil=i;
                col=j;
            }
        }
    }

    // Actualiza mapaProvisional con sus nuevas dimensiones
    mapaProvisional = sub;
}

void ComportamientoJugador::traspuesta() {
    // Inicializa matriz traspuesta
    int filas = mapaProvisional.size();
    int cols = mapaProvisional.at(filas-1).size();

    vector<vector<unsigned char>> t(cols);
    for(int i=0; i<filas; ++i)
        for(int j=0; j<cols; ++j)
            t[j].push_back(mapaProvisional[i][j]);

    // actualiza mi posicion
    swap(fil, col);

    // Actualiza mapaProvisional
    mapaProvisional = t;
}

// Rota 90 grados en sentido horario la matriz mapaProvisional (para orientar correctamente el mapa)
void ComportamientoJugador::rotar_90_grados() {
    // Evita que la posicion se actualice más de una vez (da lugar a error)
    bool posicion_actualizada=false;

    // 1. Permuta filas por capas ({1ª con ultª}, {2ª con penultª}, ..., {centro-1, centro+1})
    vector<vector<unsigned char>> aux(mapaProvisional);
    int f_j = aux.size()-1;
    for(int f_i=0; f_i<f_j; f_i++) {
        for(int c=0; c<aux[f_i].size(); ++c) {
            // Actualiza la posicion a la resultante en la nueva matriz rotada
            if( fil==f_i && col==c && !posicion_actualizada ) {
                fil = f_j;	// Solo intercambia filas (por capas), col se mantiene igual
                posicion_actualizada = true;
            } else if (f_j==fil && c==col && !posicion_actualizada) {
                fil = f_i;
                posicion_actualizada=true;
            }
            swap(aux[f_i][c], aux[f_j][c]);
        }
        f_j--;
    }

    // Actualiza mapaProvisional así como su posición en la misma
    mapaProvisional = aux;

    // 2. Calcula matriz traspuesta
    traspuesta();
}

void ComportamientoJugador::rotar_180_grados() {
    rotar_90_grados();
    rotar_90_grados();
}

void ComportamientoJugador::rotar_270_grados() {
    rotar_90_grados();
    rotar_90_grados();
    rotar_90_grados();
}

void ComportamientoJugador::transforma_matriz(Sensores sensores) {
    //assert(bien_posicionado);
    int rot = rotacion(sensores.sentido);

    // 1. Extraemos submatriz
    submatriz();

    // 2. Trabajamos sobre submatriz (aplicamos rotacion)
    switch(rot) {
        case(90):
            rotar_90_grados();
            break;
        case(180):
            rotar_180_grados();
            break;
        case(270):
            rotar_270_grados();
            break;
    }

    // 3. Insertamos la submatriz en mapaResultado (las variables fil y col contienen la
    // posición del jugador en dicha submatriz)
    for(int i=0; i<mapaProvisional.size(); ++i) {
        for(int j=0; j<mapaProvisional[i].size(); ++j) {
            if(mapaProvisional[i][j] != UNKNOWN) {
                int f_i = i-fil;
                int c_j = j-col;
                mapaResultado[sensores.posF+f_i][sensores.posC+c_j]=mapaProvisional[i][j];
            }
        }
    }

    // Eliminamos el mapa de memoria provisional (ya no lo necesitamos), y actualizamos correctamente
    // las posiciones fila y
    mapaProvisional.clear();
}

void ComportamientoJugador::reestablecer() {
    mapaProvisional.clear();
    while(!accionCompuesta.empty()) accionCompuesta.pop();
    init();
}
