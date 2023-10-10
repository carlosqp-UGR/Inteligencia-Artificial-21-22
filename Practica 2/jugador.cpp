#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>

Action ComportamientoJugador::think(Sensores sensores) {
    Action accion = actIDLE;
    actualizaEstadoActual(sensores);

    // Si se encuentra en la casilla de bikini o zapatillas las obtiene
    if(sensores.terreno[0]=='K') {
        bikini = true;
        zapatillas = false;
    } else if (sensores.terreno[0]=='D') {
        zapatillas = true;
        bikini = false;
    }

    if(sensores.nivel<3) {     // El comportamiento en los niveles 0,1,2 es completamente diferente

        if(actual.fila==sensores.destino[0] && actual.columna==sensores.destino[1]) {
            cout << endl;
            cout << "Instantes de simulacion no consumidos: " << sensores.vida << endl;
            cout << "Tiempo Consumido:" << sensores.tiempo << endl;
            cout << "Nivel Final de Bateria: " << sensores.bateria << endl << endl;
        } else {
            // Saca información por consola (opcional)
            cout << "Fila: " << actual.fila << endl;
            cout << "Col : " << actual.columna << endl;
            cout << "Ori : " << actual.orientacion << endl;
        }

        // Capturo los destinos
        cout << "sensores.num_destinos : " << sensores.num_destinos << endl;
        objetivos.clear();
        for (int i = 0; i < sensores.num_destinos; i++) {
            estado aux;
            aux.fila = sensores.destino[2*i];
            aux.columna = sensores.destino[2*i+1];
            objetivos.push_back(aux);
        }

        // Si no hay plan, construyo uno
        if(!hayPlan) {
            hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan);
        }

        if(hayPlan && !plan.empty()) {
            accion = plan.front();
            plan.pop_front();
        } else {
            cout << "no hay plan\n";
        }

    } else if(sensores.nivel==3){   // Comportamiento nivel 3
        // actualiza la información y el estado del objeto
        actualizaHistorialEstados();
        actualizaMapaResultado(sensores);
        actualizaTiempo();
        actualizaPotencialReactivo();
        actualizaPorcentajeDescubierto();

        cout << "Bateria Restante: " << sensores.bateria << endl;
        cout << "Instantes de Simulación Restantes: " << sensores.vida << endl;
        cout << "Porcentaje de Mapa Descubierto: " << porcentajeDescubierto << endl;

        // Comprueba si el plan esta vacio, bien porque no hay o por que se ha acabado
        if(plan.empty() || !estadoValido(objetivoActual))
            cancelarPlan();

        // Si no hay Plan, establece un plan de acciones
        if(!hayPlan) {
            // 1. Comprueba la cola de Objetivos Prioritarios
            if(!colaObjetivos.empty()) {
                objetivoActual = colaObjetivos.top();
                colaObjetivos.pop();
            } else if (sensores.bateria<500 && sensores.terreno[0]=='X' && !recargando) {
                plan = recargar(sensores);
                recargando = true;
                hayPlan = true;
                objetivoActual = actual;
            } else if(sensores.bateria<500 && estadoValido(obtenerMasCercana(actual, 'X'))  ) {     // Se necesita recargar
                objetivoActual = obtenerMasCercana(actual, 'X');
            } else if(porcentajeDescubierto>=17.5 || sensores.vida<2900)  {  // Objetivo Deliberativo (45)
                objetivoActual = obtenerObjetivoDeliberativo();
            } else { // Reactivo
                objetivoActual = obtenerObjetivoReactivo();
            }

            // Una vez determinado el objetivo (se debe comprobar que sea válido), determinar el plan: utilizamos dos algoritmos
            //  - Si sensores.vida < 100 : Algoritmo Búsqueda En Profundidad (menor número de acciones e instantes de Simulación)
            //  - En otro caso: Algoritmo A* (más eficiente en ejecución y en costo)
            if(estadoValido(objetivoActual) && !recargando) {
                if (sensores.vida < 100) {
                    hayPlan = pathFinding_Anchura(actual, objetivoActual, plan);
                } else {
                    hayPlan = pathFinding_A(actual, objetivoActual, plan);
                }
            }
        }

        // Si hay Plan, lo ejecuta. Se pueden dar diversos sucesos (de error) al llevar a cabo un plan, debe tenerlos en cuenta.
        accion = accionAleatoria();
        if(hayPlan) {
            // Se puede dar el caso de que la siguiente casilla al avanzar siguiendo el plan no sea válida, dos políticas: recalcular o abortar.
            // Se puede dar el caso de que el objetivo (en caso de que se observe por primera vez) esté en el agua y no tengamos bikini.
            // Se puede dar el caso de que el objetivo (en caso de que se observe por primera vez) esté en el bosque y no tengamos zapatillas.
            if(plan.front()==actFORWARD && (sensores.terreno[2]=='P' || sensores.terreno[2]=='M')) {
                // colaObjetivos.push(objetivoActual);
                cancelarPlan();
            } else if (historial.size()>5) {
                // Ejecuto la primera acción del plan cueste lo que cueste
                cout << "Estoy en un bucle y necesito salir!" << endl;
            } else if(plan.front()==actFORWARD && ( (sensores.terreno[0]!='A' && sensores.terreno[2]=='A' && !bikini) || (sensores.terreno[0]!='B' && sensores.terreno[2]=='B' && !zapatillas) ) ) {
                cancelarPlan();
            }
            // Si el plan no se ha cancelado, continúa su ejecución
            if(hayPlan && !plan.empty()) {
                accion = plan.front();
                plan.pop_front();
            }
        }

   } else if (sensores.nivel==4) { // Comportamiento nivel 4
        if(objetivos.empty())
            inicializaListaObjetivos(sensores);

        // Debido a que practicamente todo en este nivel requiere cálculos con la posicion, la principal
        // prioridad es estar bien ubicado para
        if(ultimaAccion==actWHEREIS) {
            desubicado = false;
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
        } else if(sensores.colision) {
            desubicado = true;
        }

        accion = accionAleatoria();
        if (desubicado) {
            cout << "Ubicandome..." << endl;
            accion = actWHEREIS;
        } else {
            // Actualiza la información de estado (sólamente la necesaria)
            actualizaHistorialEstados();
            actualizaMapaResultado(sensores);
            actualizaTiempo();
            actualizaPotencialReactivo();
            actualizaPorcentajeDescubierto();

            cout << "Bateria Restante: " << sensores.bateria << endl;
            cout << "Instantes de Simulación Restantes: " << sensores.vida << endl;
            cout << "Porcentaje de Mapa Descubierto: " << porcentajeDescubierto << endl;

            // Si se ha llegado al objetivo, informa de ello. (Debe eliminarlo de la lista de objetivos)
            if(actual.fila==objetivoActual.fila && actual.columna==objetivoActual.columna) {
                cout << "Otro objetivo más alcanzado!" << endl;
                eliminaObjetivo();      // Eliminar de la lista de objetivos
                cancelarPlan();
            }

            // Comprueba si el plan esta vacio, bien porque no hay o por que se ha alcanzado
            if(plan.empty() || !estadoValido(objetivoActual))
                cancelarPlan();

            // Si no hay plan, crea uno
            if(!hayPlan) {
                // 1. Establece el objetivo
                if (!colaObjetivos.empty()) {
                    objetivoActual = colaObjetivos.top();
                    colaObjetivos.pop();
                } else if (sensores.bateria < 500 && sensores.terreno[0] == 'X' && !recargando) { // Ha llegado al objetivo de recargar, genera el plan
                    plan = recargar(sensores);
                    recargando = true;
                    hayPlan = true;
                    objetivoActual = actual;
                } else if (sensores.bateria < 500 && estadoValido(obtenerMasCercana(actual, 'X'))) {     // Se necesita recargar
                    objetivoActual = obtenerMasCercana(actual, 'X');
                } else if(sensores.vida>2950 && porcentajeDescubierto<=25) {
                    objetivoActual = obtenerObjetivoReactivo();
                } else if(estadoValido(obtenerObjetivoMasCercano())){
                    objetivoActual = obtenerObjetivoMasCercano();
                }

                // 2. Si el objetivo establecido es válido, le calcula un plan
                if(estadoValido(objetivoActual) && !recargando) {
                    if (sensores.vida < 50) {
                        hayPlan = pathFinding_Anchura(actual, objetivoActual, plan);
                    } else {
                        hayPlan = pathFinding_A(actual, objetivoActual, plan);
                    }
                }
            }

            // Si hay plan continúa su ejecución, aunque debe de tener en cuenta algunas situaciones
            accion = accionAleatoria();
            if(hayPlan) {
                // Se puede dar el caso de que la siguiente casilla al avanzar siguiendo el plan no sea válida.
                // Si es un muro o un precipicio, se aborta el plan
                // Se pueden tener en cuenta más casuísticas: bikini y zapatillas
                if(plan.front()==actFORWARD) {
                    if(sensores.terreno[2]=='P' || sensores.terreno[2]=='M') {
                        cancelarPlan();
                    } else if(sensores.superficie[2]=='l' || sensores.superficie[2]=='a') {
                        accion = actIDLE;
                    } else if (historial.size()>5) {
                        // Ejecuto la primera acción del plan cueste lo que cueste
                        cout << "Estoy en un bucle y necesito salir!" << endl;
                    } else if(sensores.terreno[2]=='A' && !bikini && estadoValido(obtenerMasCercana(actual,'K'))) {
                            colaObjetivos.push(objetivoActual);
                            colaObjetivos.push(obtenerMasCercana(actual, 'K'));
                            cancelarPlan();
                    } else if(sensores.terreno[2]=='B' && !zapatillas && estadoValido(obtenerMasCercana(actual,'D'))) {
                        colaObjetivos.push(objetivoActual);
                        colaObjetivos.push(obtenerMasCercana(actual,'D'));
                        cancelarPlan();
                    }
                }

                // Si el plan no se ha cancelado, continúa su ejecución
                if(hayPlan && !plan.empty() && accion!=actIDLE) {
                    accion = plan.front();
                    plan.pop_front();
                }
            }
        }

        // ...
    }

    // Salva la última acción (la que va a realizar justo a continuación)
    ultimaAccion = accion;

    // Devuelve la accion
    return accion;
}

// Actualiza la variable actual
void ComportamientoJugador::actualizaEstadoActual(Sensores sensores) {
    if(sensores.nivel<4) {
        actual.fila = sensores.posF;
        actual.columna = sensores.posC;
        actual.orientacion = sensores.sentido;
    } else {
        if(ultimaAccion==actWHEREIS) {
            actual.fila = sensores.posF;
            actual.columna = sensores.posC;
            actual.orientacion = sensores.sentido;
        } else {
            switch (ultimaAccion) {
                case(actFORWARD):
                    switch(actual.orientacion) {
                        case (0):   actual.fila--;                      break;  // norte
                        case (1):   actual.fila--;  actual.columna++;   break;  //noreste
                        case (2):                   actual.columna++;   break;  // este
                        case (3):   actual.fila++;  actual.columna++;   break;  // sureste
                        case (4):   actual.fila++;                      break;  // sur
                        case (5):   actual.fila++;  actual.columna--;   break;  // suroeste
                        case (6):                   actual.columna--;   break;  // oeste
                        case (7):   actual.fila--;  actual.columna--;   break;  // noroeste
                    }
                    break;
                case(actTURN_R):        actual.orientacion = (actual.orientacion+2)%8; break;
                case(actTURN_L):        actual.orientacion = (actual.orientacion+6)%8; break;  // (-2)mod8=6
                case(actSEMITURN_R):    actual.orientacion = (actual.orientacion+1)%8; break;
                case(actSEMITURN_L):    actual.orientacion = (actual.orientacion+7)%8; break; // (-1)mod8=7
            }
        }
    }
}

bool ComportamientoJugador::pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan) {
    bool path = false;
	switch (level) {
        case (0):
            cout << "Demo\n";
            estado objetivo0;
            objetivo0 = objetivos.front();
            cout << "fila: " << objetivo0.fila << " col:" << objetivo0.columna << endl;
            return pathFinding_Profundidad(origen, objetivo0, plan);
            break;
        case (1):
            cout << "Optimo numero de acciones\n";
            estado objetivo1;
            objetivo1 = objetivos.front();
            cout << "fila: " << objetivo1.fila << " col:" << objetivo1.columna << endl;
            return pathFinding_Anchura(origen, objetivo1, plan);
            break;
        case 2:
            // Incluir aqui la llamada al busqueda de costo uniforme/A*
            cout << "Optimo en coste (coste uniforme)\n";
            estado objetivo2;
            objetivo2 = objetivos.front();
            cout << "fila: " << objetivo2.fila << " col:" << objetivo2.columna << endl;
            // return pathFinding_CosteUniforme(origen, objetivo2, plan);
            return pathFinding_A(origen, objetivo2, plan);
            cout << "No implementado aun\n";
            break;
        case 3:
            cout << "Reto 1: Descubrir el mapa\n";
            // Incluir aqui la llamada al algoritmo de busqueda para el Reto 1
            cout << "No implementado aun\n";
            break;
        case 4:
            cout << "Reto 2: Maximizar objetivos\n";
            // Incluir aqui la llamada al algoritmo de busqueda para el Reto 2
            cout << "No implementado aun\n";
            break;
	}
	return false;
}

// Sacar por la consola la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan)
{
    auto it = plan.begin();
    while (it != plan.end())
    {
        if (*it == actFORWARD)
        {
            cout << "A ";
        }
        else if (*it == actTURN_R)
        {
            cout << "D ";
        }
        else if (*it == actSEMITURN_R)
        {
            cout << "d ";
        }
        else if (*it == actTURN_L)
        {
            cout << "I ";
        }
        else if (*it == actSEMITURN_L)
        {
            cout << "I ";
        }
        else
        {
            cout << "- ";
        }
        it++;
    }
    cout << endl;
}

// Funcion auxiliar para poner a 0 todas las casillas de una matriz
void AnularMatriz(vector<vector<unsigned char>> &m)
{
    for (int i = 0; i < m[0].size(); i++)
    {
        for (int j = 0; j < m.size(); j++)
        {
            m[i][j] = 0;
        }
    }
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan)
{
    AnularMatriz(mapaConPlan);
    estado cst = st;

    auto it = plan.begin();
    while (it != plan.end())
    {
        if (*it == actFORWARD)
        {
            switch (cst.orientacion)
            {
                case 0:
                    cst.fila--;
                    break;
                case 1:
                    cst.fila--;
                    cst.columna++;
                    break;
                case 2:
                    cst.columna++;
                    break;
                case 3:
                    cst.fila++;
                    cst.columna++;
                    break;
                case 4:
                    cst.fila++;
                    break;
                case 5:
                    cst.fila++;
                    cst.columna--;
                    break;
                case 6:
                    cst.columna--;
                    break;
                case 7:
                    cst.fila--;
                    cst.columna--;
                    break;
            }
            mapaConPlan[cst.fila][cst.columna] = 1;
        }
        else if (*it == actTURN_R)
        {
            cst.orientacion = (cst.orientacion + 2) % 8;
        }
        else if (*it == actSEMITURN_R)
        {
            cst.orientacion = (cst.orientacion + 1) % 8;
        }
        else if (*it == actTURN_L)
        {
            cst.orientacion = (cst.orientacion + 6) % 8;
        }
        else if (*it == actSEMITURN_L)
        {
            cst.orientacion = (cst.orientacion + 7) % 8;
        }
        it++;
    }
}

int ComportamientoJugador::interact(Action accion, int valor) {
    return false;
}

//---------------------- Implementación de métodos auxiliares ---------------------------

// Dado el codigo en caracter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla) {
    if (casilla == 'P' or casilla == 'M') return true;
    else return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st) {
    int fil = st.fila, col = st.columna;
    // calculo cual es la casilla de delante del agente
    switch (st.orientacion) {
        case 0: fil--; break;
        case 1: fil--; col++; break;
        case 2: col++; break;
        case 3: fil++; col++; break;
        case 4: fil++; break;
        case 5: fil++; col--; break;
        case 6: col--; break;
        case 7: fil--; col--; break;
    }

    // Compruebo que no me salgo fuera del rango del mapa
    if (fil < 0 or fil >= mapaResultado.size())
        return true;
    if (col < 0 or col >= mapaResultado[0].size())
        return true;

    // Miro si en esa casilla hay un obstaculo infranqueable
    if (!EsObstaculo(mapaResultado[fil][col])) {
        // No hay obstaculo, actualizo el parametro st poniendo la casilla de delante.
        st.fila = fil;
        st.columna = col;
        return false;
    } else {
        return true;
    }
}

//---------------------- Implementación de métodos, algoritmos y estructuras para los niveles 0 y 1 ---------------------------
struct nodo
{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados {
	bool operator()(const estado &a, const estado &n) const {
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

// Implementación de la busqueda en profundidad
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {

    // Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; // Lista de Cerrados
	stack<nodo> Abiertos;				  // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
			Abiertos.push(hijoSEMITurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

        // Comprueba y extrae los nodos de abiertos que están en cerrados
        while( Cerrados.find(Abiertos.top().st) != Cerrados.end() ) Abiertos.pop();

        // Tomo el siguiente valor de la Abiertos
        if (!Abiertos.empty())  current = Abiertos.top();
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

// Implementación de la busqueda en anchura.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {

    // Borro la lista
    cout << "Calculando plan\n";
    plan.clear();
    set<estado, ComparaEstados> Cerrados; // Lista de Cerrados (explorados/visitados)
    queue<nodo> Abiertos;				  // Lista de Abiertos (frontera, por visitar/no explorados), FIFO

    // Nodo a procesar: inicialmente el nodo origen, posteriormente se irá extrayendo de la lista de abiertos
    nodo current;
    current.st = origen;
    current.secuencia.empty(); // Inicialmente la secuencia de acciones está vacía (ya se encuentra en dicho nodo)

    Abiertos.push(current); // Únicamente para la primera iteración

    // Mientras queden nodos sin explorar y no hayamos encontrado la solución
    while (!Abiertos.empty() && !(current.st.fila==destino.fila && current.st.columna==destino.columna)) {
        // Saca el nodo actual de la lista de no explorados y lo inserta en la lista de visitados
        Abiertos.pop();
        Cerrados.insert(current.st);

        // Genera todos los posibles descendientes como copias del actual con ligeras modificaciones:
        // Cada uno almacena la secuencia de acciones del actual, con su respectiva modificacion (última acción para llegar a ese hijo)

        // Generar descendiente de girar a la derecha 90 grados
        nodo hijoTurnR = current;
        hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
        if (Cerrados.find(hijoTurnR.st) == Cerrados.end()) { // Comprueba que el hijo no esté en explorados (evita repeticiones)
            hijoTurnR.secuencia.push_back(actTURN_R);
            Abiertos.push(hijoTurnR);
        }

        // Generar descendiente de girar a la derecha 45 grados y lo añade a Abiertos
        nodo hijoSEMITurnR = current;
        hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
        if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end()) {
            hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
            Abiertos.push(hijoSEMITurnR);
        }

        // Generar descendiente de girar a la izquierda 90 grados
        nodo hijoTurnL = current;
        hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
        if (Cerrados.find(hijoTurnL.st) == Cerrados.end()) {
            hijoTurnL.secuencia.push_back(actTURN_L);
            Abiertos.push(hijoTurnL);
        }

        // Generar descendiente de girar a la izquierda 45 grados
        nodo hijoSEMITurnL = current;
        hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
        if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end()) {
            hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
            Abiertos.push(hijoSEMITurnL);
        }

        // Generar descendiente de avanzar
        nodo hijoForward = current;
        if (!HayObstaculoDelante(hijoForward.st)) { // Comprueba si es viable el camino
            if (Cerrados.find(hijoForward.st) == Cerrados.end()) {
                hijoForward.secuencia.push_back(actFORWARD);
                Abiertos.push(hijoForward);
            }
        }

        // Comprueba y extrae los nodos de abiertos que están en cerrados
        while( Cerrados.find(Abiertos.front().st) != Cerrados.end() ) Abiertos.pop();

        // Tomo el siguiente valor de la Abiertos
        if (!Abiertos.empty())  current = Abiertos.front();

    } // Fin del algoritmo: acaba si ha visitado todos los posibles nodos o si se encuentra en el nodo destino

    cout << "Terminada la busqueda\n";

    // Carga el resultado de la búsqueda en la memoria del agente
    if (current.st.fila == destino.fila and current.st.columna == destino.columna) {
        cout << "Cargando el plan\n";
        plan = current.secuencia;
        cout << "Longitud del plan: " << plan.size() << endl;
        PintaPlan(plan);	// Imprime el plan de acciones por consola
        VisualizaPlan(origen, plan);	// Muestra el plan en el mapa
        return true;
    } else cout << "No encontrado plan\n";

    return false;
}

//---------------------- Implementación de métodos y estructuras para el nivel 2  ---------------------------

// Ligera modificación de estructura necesaria para busqueda por coste uniforme, se debe de tener en cuenta el coste
struct nodoCoste {
    // Informacion de estado
    estado st;
    bool zapatillas;
    bool bikini;
    int coste;

    // Secuencia de acciones hasta llegar a ese estado
    list<Action> secuencia;
};

struct ComparaCoste {
    bool operator()(const nodoCoste &a, const nodoCoste &b) const {
        return a.coste>b.coste;
    }
};

// Implementación de la busqueda en Coste Uniforme.
bool ComportamientoJugador::pathFinding_CosteUniforme(const estado &origen, const estado &destino, list<Action> &plan) {

    // Borro la lista
    cout << "Calculando plan\n";
    plan.clear();
    set<estado, ComparaEstados> Cerrados;                          // Lista de Cerrados (explorados/visitados)
    priority_queue<nodoCoste, vector<nodoCoste>, ComparaCoste> Abiertos;    // Lista de Abiertos (frontera, por visitar/no explorados), FIFO

    // Nodo a procesar: inicialmente el nodo origen, posteriormente se irá extrayendo de la lista de abiertos
    nodoCoste current;
    current.st = origen;
    current.secuencia.empty(); // Inicialmente la secuencia de acciones está vacía (ya se encuentra en dicho nodo)
    current.zapatillas = zapatillas;
    current.bikini = bikini;
    current.coste = 0;  // El coste inicial es 0
    Abiertos.push(current); // Únicamente para la primera iteración
    
    // Mientras queden nodos sin explorar y no hayamos encontrado la solución
    while (!Abiertos.empty() && !(current.st.fila==destino.fila && current.st.columna==destino.columna)) {
        // Saca el nodo actual de la lista de no explorados y lo inserta en la lista de visitados
        Abiertos.pop();
        Cerrados.insert(current.st);

        unsigned char currentCasilla = mapaResultado[current.st.fila][current.st.columna];

        // Comprueba si en el estado se encuentra en la casilla de zapatillas o de bikini para añadirlas
        if(currentCasilla=='K') {
            current.bikini = true;
            current.zapatillas = false;
        } else if (currentCasilla=='D') {
            current.bikini = false;
            current.zapatillas = true;
        }

        // Genera todos los posibles descendientes como copias del actual con ligeras modificaciones:
        // Cada uno almacena la secuencia de acciones del actual, con su respectiva modificacion
        //  - Información de estado nueva (posicion, orientacion, bikini, zapatillas, coste)
        //  - Ultima accion para llegar a ese hijo

        // Generar descendiente de girar a la derecha 90 grados
        nodoCoste hijoTurnR = current;
        hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
        hijoTurnR.coste += c(actTURN_R, currentCasilla, current.bikini, current.zapatillas); // Información Heurística
        if (Cerrados.find(hijoTurnR.st) == Cerrados.end()) { // Comprueba que el hijo no esté en explorados (evita repeticiones)
            hijoTurnR.secuencia.push_back(actTURN_R);
            Abiertos.push(hijoTurnR);
        }

        // Generar descendiente de girar a la derecha 45 grados y lo añade a Abiertos
        nodoCoste hijoSEMITurnR = current;
        hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
        hijoSEMITurnR.coste += c(actSEMITURN_R, currentCasilla, current.bikini, current.zapatillas);
        if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end()) {
            hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
            Abiertos.push(hijoSEMITurnR);
        }

        // Generar descendiente de girar a la izquierda 90 grados
        nodoCoste hijoTurnL = current;
        hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
        hijoTurnL.coste += c(actTURN_L, currentCasilla, current.bikini, current.zapatillas);
        if (Cerrados.find(hijoTurnL.st) == Cerrados.end()) {
            hijoTurnL.secuencia.push_back(actTURN_L);
            Abiertos.push(hijoTurnL);
        }

        // Generar descendiente de girar a la izquierda 45 grados
        nodoCoste hijoSEMITurnL = current;
        hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
        hijoSEMITurnL.coste += c(actSEMITURN_L, currentCasilla, current.bikini, current.zapatillas);
        if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end()) {
            hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
            Abiertos.push(hijoSEMITurnL);
        }

        // Generar descendiente de avanzar
        nodoCoste hijoForward = current;
        if (!HayObstaculoDelante(hijoForward.st)) { // Comprueba si es viable el camino (la funcion de obstaculo actualiza la posicion)
            hijoForward.coste += c(actFORWARD, currentCasilla, current.bikini, current.zapatillas);
            if (Cerrados.find(hijoForward.st) == Cerrados.end()) {
                hijoForward.secuencia.push_back(actFORWARD);
                Abiertos.push(hijoForward);
            }
        }

        // Comprueba y extrae los nodos de abiertos que están en cerrados
        while( Cerrados.find(Abiertos.top().st) != Cerrados.end() ) Abiertos.pop();

        // Tomo el siguiente valor de la Abiertos
        if (!Abiertos.empty())  current = Abiertos.top();

    } // Fin del algoritmo: acaba si ha visitado todos los posibles nodos o si se encuentra en el nodo destino

    cout << "Terminada la busqueda\n";

    // Carga el resultado de la búsqueda en la memoria del agente
    if (current.st.fila == destino.fila and current.st.columna == destino.columna) {
        cout << "Cargando el plan...\n";
        plan = current.secuencia;
        cout << "Longitud del plan: " << plan.size() << endl;
        cout << "Coste (en batería) del plan: " << current.coste << endl;
        cout << "Plan: " << endl;
        PintaPlan(plan);	// Imprime el plan de acciones por consola
        VisualizaPlan(origen, plan);	// Muestra el plan en el mapa
        return true;
    } else cout << "No encontrado plan\n";

    return false;
}

// -------------------------------------------- Implementación del algoritmo A* ---------------------------------------

// Posibles funciones heurísticas
// ADMISIBLE
int ComportamientoJugador::h(const estado &st, const estado &obj) {
    return max(abs(st.fila-obj.fila),abs(st.columna-obj.columna) );
}

// NO ADMISIBLE
int ComportamientoJugador::distanciaManhattan(const estado &st, const estado &obj) {
    return ( abs(st.fila-obj.fila) + abs(st.columna-obj.columna)  );
}

int ComportamientoJugador::distanciaEuclidea(const estado &st, const estado &obj) {
    return ceil(sqrt((st.fila-obj.fila)*(st.fila-obj.fila)+(st.columna-obj.columna)*(st.columna-obj.columna)));
}

// Estructuras necesarias para la implementación del algoritmo
struct nodoA {
    // Informacion de estado
    estado st;
    bool zapatillas;
    bool bikini;
    int coste;
    int heuristica;

    // Secuencia de acciones hasta llegar a ese estado
    list<Action> secuencia;
};

struct ComparaA {
    bool operator()(const nodoA &a, const nodoA &b) const {
        return (a.coste+a.heuristica)>(b.coste+b.heuristica);
    }
};

// Algoritmo A*
bool ComportamientoJugador::pathFinding_A(const estado &origen, const estado &destino, list<Action> &plan) {

    // Borro la lista
    cout << "Calculando plan\n";
    plan.clear();
    set<estado, ComparaEstados> Cerrados;                          // Lista de Cerrados (explorados/visitados)
    priority_queue<nodoA, vector<nodoA>, ComparaA> Abiertos;    // Lista de Abiertos (frontera, por visitar/no explorados), FIFO

    // Nodo a procesar: inicialmente el nodo origen, posteriormente se irá extrayendo de la lista de abiertos
    nodoA current;
    current.st = origen;
    current.secuencia.empty(); // Inicialmente la secuencia de acciones está vacía (ya se encuentra en dicho nodo)
    current.zapatillas = zapatillas;
    current.bikini = bikini;
    current.coste = 0;  // El coste inicial es 0
    current.heuristica = 0; // ¿?
    Abiertos.push(current); // Únicamente para la primera iteración

    // Mientras queden nodos sin explorar y no hayamos encontrado la solución
    while (!Abiertos.empty() && !(current.st.fila==destino.fila && current.st.columna==destino.columna)) {
        // Saca el nodo actual de la lista de no explorados y lo inserta en la lista de visitados
        Abiertos.pop();
        Cerrados.insert(current.st);

        unsigned char currentCasilla = mapaResultado[current.st.fila][current.st.columna];

        // Comprueba si en el estado se encuentra en la casilla de zapatillas o de bikini para añadirlas
        if(currentCasilla=='K') {
            current.bikini = true;
            current.zapatillas = false;
        } else if (currentCasilla=='D') {
            current.bikini = false;
            current.zapatillas = true;
        }

        // Genera todos los posibles descendientes como copias del actual con ligeras modificaciones:
        // Cada uno almacena la secuencia de acciones del actual, con su respectiva modificacion
        //  - Información de estado nueva (posicion, orientacion, bikini, zapatillas, coste)
        //  - Ultima accion para llegar a ese hijo

        // Generar descendiente de girar a la derecha 90 grados
        nodoA hijoTurnR = current;
        hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
        hijoTurnR.coste += c(actTURN_R, currentCasilla, current.bikini, current.zapatillas); // Información Heurística
        hijoTurnR.heuristica = h(hijoTurnR.st, destino);
        if (Cerrados.find(hijoTurnR.st) == Cerrados.end()) { // Comprueba que el hijo no esté en explorados (evita repeticiones)
            hijoTurnR.secuencia.push_back(actTURN_R);
            Abiertos.push(hijoTurnR);
        }

        // Generar descendiente de girar a la derecha 45 grados y lo añade a Abiertos
        nodoA hijoSEMITurnR = current;
        hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
        hijoSEMITurnR.coste += c(actSEMITURN_R, currentCasilla, current.bikini, current.zapatillas);
        hijoSEMITurnR.heuristica = h(hijoSEMITurnR.st, destino);
        if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end()) {
            hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
            Abiertos.push(hijoSEMITurnR);
        }

        // Generar descendiente de girar a la izquierda 90 grados
        nodoA hijoTurnL = current;
        hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
        hijoTurnL.coste += c(actTURN_L, currentCasilla, current.bikini, current.zapatillas);
        hijoTurnL.heuristica = h(hijoTurnL.st, destino);
        if (Cerrados.find(hijoTurnL.st) == Cerrados.end()) {
            hijoTurnL.secuencia.push_back(actTURN_L);
            Abiertos.push(hijoTurnL);
        }

        // Generar descendiente de girar a la izquierda 45 grados
        nodoA hijoSEMITurnL = current;
        hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
        hijoSEMITurnL.coste += c(actSEMITURN_L, currentCasilla, current.bikini, current.zapatillas);
        hijoSEMITurnL.heuristica = h(hijoSEMITurnL.st, destino);
        if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end()) {
            hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
            Abiertos.push(hijoSEMITurnL);
        }

        // Generar descendiente de avanzar
        nodoA hijoForward = current;
        if (!HayObstaculoDelante(hijoForward.st)) { // Comprueba si es viable el camino (la funcion de obstaculo actualiza la posicion)
            hijoForward.coste += c(actFORWARD, currentCasilla, current.bikini, current.zapatillas);
            hijoForward.heuristica = h(hijoForward.st, destino);
            if (Cerrados.find(hijoForward.st) == Cerrados.end()) {
                hijoForward.secuencia.push_back(actFORWARD);
                Abiertos.push(hijoForward);
            }
        }

        // Comprueba y extrae los nodos de abiertos que están en cerrados
        while( Cerrados.find(Abiertos.top().st) != Cerrados.end() ) Abiertos.pop();

        // Tomo el siguiente valor de la Abiertos
        if (!Abiertos.empty())  current = Abiertos.top();

    } // Fin del algoritmo: acaba si ha visitado todos los posibles nodos o si se encuentra en el nodo destino

    cout << "Terminada la busqueda\n";

    // Carga el resultado de la búsqueda en la memoria del agente
    if (current.st.fila == destino.fila and current.st.columna == destino.columna) {
        cout << "Cargando el plan\n";
        plan = current.secuencia;
        cout << "Longitud del plan: " << plan.size() << endl;
        cout << "Coste (en batería) del plan: " << current.coste << endl;
        PintaPlan(plan);	// Imprime el plan de acciones por consola
        VisualizaPlan(origen, plan);	// Muestra el plan en el mapa
        return true;
    } else cout << "No encontrado plan\n";

    return false;
}

//---------------------- Implementación de métodos para actualización del mapa resultado (nivel 4)  ---------------------------

int ComportamientoJugador::profundidad(int orientacion, int i) {
    int prof;
    if(orientacion%2==0) { 	// Norte, Este, Sur, Oeste
        if(i==0) prof = 0;
        else if(1<=i && i<=3) prof = 1;
        else if(4<=i && i<=8) prof = 2;
        else if(9<=i && i<=15) prof = 3;
    } else { 				// Noreste, Sureste, Suroeste, Noroeste
        if(i==0 || i==3 || i==8 || i==15) prof=0;
        else if(i==1 || i==2 || i==7 || i==14) prof=1;
        else if(i==4 || i==5 || i==6 || i==13) prof=2;
        else if(i==9 || i==10 || i==11 || i==12) prof=3;
    }
    return prof;
}

int ComportamientoJugador::lateralidad(int orientacion, int i) {
    int lat;
    if(orientacion%2==0) { 	// Norte, Este, Sur, Oeste
        if(i==0) lat = 0;
        else if(1<=i && i<=3) lat = i-2;
        else if(4<=i && i<=8) lat = i-6;
        else if(9<=i && i<=15) lat = i-12;
    } else { 				// Noreste, Sureste, Suroeste, Noroeste
        int factor=1;	// La lateralidad es a la derecha
        if(orientacion==3 || orientacion==7) factor=-1; // La lateralidad es a la izquierda
        if(i==0 || i==1 || i==4 || i==9) lat = 0;
        else if(i==3 || i==2 || i==5 || i==10) lat = 1;
        else if(i==8 || i==7 || i==6 || i==11) lat = 2;
        else if(i==15 || i==14 || i==13 || i==12) lat = 3;
        lat*=factor;
    }
    return lat;
}

// Actualiza el mapa resultado según su visión
void ComportamientoJugador::actualizaMapaResultado(Sensores sensores) {
    // Esta función se invoca únicamente en el nivel 4
    if (sensores.nivel==3 || (sensores.nivel==4 && !desubicado)) {  // Justo en el inicio del juego, no conocemos la orientación ni la posición
        for (int i = 0; i < sensores.terreno.size(); i++) {
            int fila, col;
            switch (actual.orientacion) {
                case 0: fila = actual.fila - profundidad(actual.orientacion, i); col = actual.columna + lateralidad(actual.orientacion, i); break;  // Norte
                case 2: fila = actual.fila + lateralidad(actual.orientacion, i); col = actual.columna + profundidad(actual.orientacion, i); break;  // Este
                case 4: fila = actual.fila + profundidad(actual.orientacion, i); col = actual.columna - lateralidad(actual.orientacion, i); break;  // Sur
                case 6: fila = actual.fila - lateralidad(actual.orientacion, i); col = actual.columna - profundidad(actual.orientacion, i); break;  // Oeste

                case 1: fila = actual.fila - profundidad(actual.orientacion, i); col = actual.columna + lateralidad(actual.orientacion, i); break;  // Noreste                                                                                                                           // Noreste
                case 7: fila = actual.fila + lateralidad(actual.orientacion, i); col = actual.columna - profundidad(actual.orientacion, i); break;  // Noroeste

                case 3: fila = actual.fila - lateralidad(actual.orientacion, i); col = actual.columna + profundidad(actual.orientacion, i); break;  // Sureste                                                                                                                    // Sureste
                case 5: fila = actual.fila + profundidad(actual.orientacion, i); col = actual.columna - lateralidad(actual.orientacion, i); break;  // Suroeste
            }

            // Comprueba el caso en el que la visión vaya más alla del borde del mapa (no debería de ocurrir)
            if ((0 <= fila && fila < mapaResultado.size()) && (0 <= col && col < mapaResultado.size())) {
                if (mapaResultado[fila][col] == '?')    // Si la casilla era desconocida, la actualiza
                    mapaResultado[fila][col] = sensores.terreno[i];
            }
        }
    }
}

// Devuelve el coste de batería
int ComportamientoJugador::c(Action accion, unsigned char casilla, bool tiene_bikini, bool tiene_zapatillas) {
    int coste=0;
    switch (accion) {
        case(actWHEREIS): coste = 200; break;

        case(actIDLE): coste = 0; break;

        case(actFORWARD):
            switch (casilla) {
                case('A'):
                    if(tiene_bikini) coste = 10;
                    else coste = 200;
                    break;
                case('B'):
                    if(tiene_zapatillas) coste = 15;
                    else coste = 100;
                    break;
                case('T'): coste = 2; break;
                case('?'):
                    coste = 200; // En caso de que no se conozca la casilla, el coste es máximo (estimación, puesto que siempre va a ser menor o igual)
                    break;
                default: coste = 1;
            }
            break;

        case(actTURN_R):
        case(actTURN_L):
            switch (casilla) {
                case('A'):
                    if(tiene_bikini) coste = 5;
                    else coste = 500;
                    break;
                case('B'):
                    if(tiene_zapatillas) coste = 1;
                    else coste = 3;
                    break;
                case('T'): coste = 2; break;
                case('?'):
                    coste = 500; // En caso de que no se conozca la casilla, el coste es máximo (estimación, puesto que siempre va a ser menor o igual)
                    break;
                default: coste = 1;
            }
            break;

        case(actSEMITURN_L):
        case(actSEMITURN_R):
            switch (casilla) {
                case('A'):
                    if(tiene_bikini) coste = 2;
                    else coste = 300;
                    break;
                case('B'):
                    if(tiene_zapatillas) coste = 1;
                    else coste = 2;
                    break;
                case('?'):
                    coste = 500; // En caso de que no se conozca la casilla, el coste es máximo (estimación, puesto que siempre va a ser menor o igual)
                    break;
                default: coste = 1;
            }
            break;
    }

    return coste;
}

//---------------------- Implementación de métodos para el nivel 3  ---------------------------
unsigned int ComportamientoJugador::potencialReactivo(int fila, int columna) {
    unsigned char casilla = mapaResultado[fila][columna];
    unsigned int pt = 0;
    switch (casilla) {
        case('A'):
            if(bikini) pt = 5;
            else pt = 25;
            break;
        case('B'):
            if(zapatillas) pt = 5;
            else pt = 15;
            break;
        case('T'): pt = 2; break;
        case('?'): pt = 20; break;
        case('M'):
        case('P'): pt = INF; break;
        default: pt = 1;
    }
    return pt;
}

void ComportamientoJugador::actualizaTiempo() {
    const int Visitado = 2;
    const int Observado = 1;
    // Primero actualiza la casilla en la que se encuentra (sólo una por iteración)
    tiempo[actual.fila][actual.columna].first = estadoCasilla::VISITADA;
    for(int i=0; i<mapaResultado.size(); ++i) {
        for(int j=0; j<mapaResultado.size(); ++j) {
            // Actualiza el estado de aquellas casillas que ha observado por primera vez
            if(tiempo[i][j].first==estadoCasilla::DESCONOCIDA && mapaResultado[i][j]!='?') {
                tiempo[i][j].first = estadoCasilla::OBSERVADA;
            }

            // Le suma el valor correspondiente a cada casilla
            if(tiempo[i][j].first==estadoCasilla::OBSERVADA) tiempo[i][j].second += Observado;
            else if (tiempo[i][j].first==estadoCasilla::VISITADA) tiempo[i][j].second += Visitado;
        }
    }

}

// Se debe de llamar a esta función tras actualizar mapaResultado y actualizar tiempo
void ComportamientoJugador::actualizaPotencialReactivo() {
    int centro = floor(N/2);
    for (int fil=0; fil<N; ++fil) {
        int fila_real = actual.fila + (fil-centro);
        for (int col=0; col<N; ++col) {
            int col_real = actual.columna + (col-centro);
            if(0<=fila_real && fila_real<mapaResultado.size() && 0<=col_real && col_real<mapaResultado.size())  // Evita salirse del mapa
                potencial[fil][col] = potencialReactivo(fila_real, col_real) + tiempo[fila_real][col_real].second;
            else potencial[fil][col] = INF; // En el caso de que se salga del mapa, el potencial es infinito (no va nunca hacia alli)
        }
    }
}

// No permite devolver la casilla actual
estado ComportamientoJugador::obtenerObjetivoReactivo() {
    list<estado> minimos;
    int centro = floor(potencial.size()/2);
    int min = INF;
    for(int fil=0; fil<potencial.size(); ++fil) {
        int fila_real = actual.fila + (fil-centro);
        for (int col = 0; col < potencial[fil].size(); ++col) {
            int col_real = actual.columna + (col-centro);
            if (0 <= fila_real && fila_real < mapaResultado.size() && 0 <= col_real && col_real < mapaResultado.size()) {  // Evita salirse del mapa
                estado st;
                st.fila = fila_real;
                st.columna = col_real;
                if(fila_real!=actual.fila && col_real!=actual.columna ) {
                    if (potencial[fil][col] < min) {
                        min = potencial[fil][col];
                        minimos.clear();
                        minimos.push_back(st);
                    } else if (potencial[fil][col] == min) {
                        minimos.push_back(st);
                    }
                }
            }
        }
    }

    estado min_st;
    min_st.fila = -1;
    min_st.columna = -1;
    if(!minimos.empty()) {
        // Ahora, en caso de empate, selecciona uno al azar
        int min_a_elegir = rand()%minimos.size();
        // Movemos el iterador hasta el estado seleccionado y lo devolvemos
        list<estado>::const_iterator it = minimos.cbegin();
        for(int i=0; i<min_a_elegir; ++i) ++it;
        min_st = *it;
    }

    return min_st;
}

void ComportamientoJugador::actualizaPorcentajeDescubierto() {
    int cont=0; int total_casillas=0;
    for(int i=0; i<mapaResultado.size(); ++i) {
        for(int j=0; j<mapaResultado.size(); ++j) {
            if(mapaResultado[i][j]!='?') cont++;
            total_casillas++;
        }
    }
    porcentajeDescubierto = 100 * ((double) cont)/total_casillas;
}

void ComportamientoJugador::cancelarPlan() {
    objetivoActual.fila = -1;
    objetivoActual.columna = -1;
    plan.clear();
    hayPlan = false;
    recargando = false;
    indiceObjetivoActual = -1;
}

// Una accion aleatoria diferente de avanzar y de posicionarse
Action ComportamientoJugador::accionAleatoria() {
    Action accion;
    int num = rand()%5 + 1;
    switch (num) {
        case(1): accion = actTURN_R; break;
        case(2): accion = actTURN_L; break;
        case(3): accion = actSEMITURN_R; break;
        case(4): accion = actSEMITURN_L; break;
        case(5): accion = actIDLE; break;
    }
    return accion;
}

bool ComportamientoJugador::estadoValido(const estado &st) {
    bool valido = true;
    if ( !(0<=st.fila && st.fila<=mapaResultado.size() && 0<=st.columna && st.columna<=mapaResultado.size()) )  // Posición fuera de rango
        valido = false;
    else if (mapaResultado[st.fila][st.columna]=='P' || mapaResultado[st.fila][st.columna]=='M')    // Estado no válido por ser muro o precipicio
        valido = false;
    return valido;
}

estado ComportamientoJugador::obtenerObjetivoDeliberativo() {
    // Definir política de comportamiento. Podría obtener el objetivo más lejano o el más cercano (función heurística h)
    list<estado> candidatos;
    int minDistancia = INF ;

    // Si no se encuentra en un bucle, el objetivo es el más cercano
    if(historial.size()<4) {
        // (a) Política de selección del objetivo: el DESCONOCIDO más cercano
        estado aux;
        for(int i=0; i<mapaResultado.size(); ++i) {
            aux.fila = i;
            for(int j=0; j<mapaResultado.size();  ++j) {
                aux.columna = j;
                if (tiempo[i][j].first==estadoCasilla::DESCONOCIDA) {
                    if (h(actual, aux)==minDistancia) {
                        candidatos.push_back(aux);
                    } else if (h(actual, aux) < minDistancia) {
                        candidatos.clear();
                        candidatos.push_back(aux);
                        minDistancia = h(actual, aux);
                    }
                }
            }
        }

    } else {    // Si se encuentra en un bucle, el objetivo es aleatorio
        // (b) Política de selección del objetivo: aleatorio (pero con distancia menor h=20)
        estado st;
        for(int i=0; i<mapaResultado.size(); ++i) {
            st.fila = i;
            for(int j=0; j<mapaResultado.size();  ++j) {
                st.columna = j;
                if (tiempo[i][j].first==estadoCasilla::DESCONOCIDA && h(actual, st)<25) {
                    candidatos.push_back(st);
                }
            }
        }
    }

    // Selección del objetivo. De entre los candidatos, al azar.
    estado objetivo;

    if(candidatos.empty()) {
        estado objetivo;
        objetivo.fila = -1;
        objetivo.columna = -1;
    } else {
        int obj_a_elegir = rand()%candidatos.size();
        // Movemos el iterador hasta el estado seleccionado y lo devolvemos
        list<estado>::const_iterator it = candidatos.cbegin();
        for(int i=0; i<obj_a_elegir; ++i) ++it;
        objetivo = *it;
    }

    return objetivo;
}


estado ComportamientoJugador::obtenerMasCercana(const estado &st, unsigned char tipoCasilla) {
    estado masCercano;
    masCercano.fila = -1;
    masCercano.columna = -1;
    int distancia = INF;

    estado p;
    for (int i = 0; i < mapaResultado.size(); ++i) {
        p.fila = i;
        for (int j = 0; j < mapaResultado.size(); ++j) {
            p.columna = j;
            if(mapaResultado[i][j] == tipoCasilla && h(st, p)<distancia) {
                masCercano = p;
                distancia = h(st, masCercano);
            }
        }
    }

    return masCercano;
}

void ComportamientoJugador::actualizaHistorialEstados() {
    if(historial.empty()) {
        historial.push_back(actual);
    } else if (actual.fila == historial.back().fila && actual.columna == historial.back().columna ) {
        historial.push_back(actual);
    } else {
        historial.clear();
        historial.push_back(actual);
    }
}

list<Action> ComportamientoJugador::recargar(Sensores sensores) {
    int a = 0.1*sensores.vida;
    int b = (3000-sensores.bateria)/10;
    int instantes_a_consumir = min(a,b);
    list<Action> recarga;
    for(int i=0; i<instantes_a_consumir; ++i)
        recarga.push_back(actIDLE);
    return recarga;
}

estado ComportamientoJugador::obtenerObjetivoMasCercano() {
    estado minimo;
    indiceObjetivoActual = -1;
    minimo.fila = -1;
    minimo.columna = -1;
    // Busca el objetivo mínimo
    if(!objetivos.empty()) {
        int minDistancia = INF ;
        int iteraciones = 0;
        for(list<estado>::const_iterator it = objetivos.cbegin(); it!=objetivos.cend(); ++it) {
            if(h(actual, *it) < minDistancia) {
                minDistancia = h(actual, *it);
                minimo = *it;
                indiceObjetivoActual = iteraciones;
            }
            iteraciones++;
        }
    }
    return minimo;
}

void ComportamientoJugador::inicializaListaObjetivos(Sensores sensores){
    estado aux;
    for(int i=0; i<sensores.num_destinos; ++i) {
        aux.fila = sensores.destino[2*i];
        aux.columna = sensores.destino[2*i+1];
        objetivos.push_back(aux);
    }
}

void ComportamientoJugador::eliminaObjetivo() {
    // Los objetivos se enumeran desde 0
    if(indiceObjetivoActual>=0 && !objetivos.empty()) {
        list<estado>::iterator it = objetivos.begin();
        for(int i=0; i<indiceObjetivoActual; ++i) ++it;
        objetivos.erase(it);
    }
    indiceObjetivoActual = -1;
}