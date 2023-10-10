# include "AIPlayer.h"
# include "Parchis.h"

const double masinf = 9999999999.0, menosinf = -9999999999.0;
const double gana = masinf - 1, pierde = menosinf + 1;
const int num_pieces = 4;
const int PROFUNDIDAD_MINIMAX = 4;  // Umbral maximo de profundidad para el metodo MiniMax
const int PROFUNDIDAD_ALFABETA = 6; // Umbral maximo de profundidad para la poda Alfa_Beta

void AIPlayer::think(color & c_piece, int & id_piece, int & dice) const{

    double alpha = menosinf;
    double beta = masinf;
    switch(id) {
        case(0):
            // MiniMax(*actual, id, 0, PROFUNDIDAD_MINIMAX, c_piece, id_piece, dice, ValoracionTest);
            PodaAlfaBeta(*actual, id, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, menosinf, masinf, myHeuristic1);
            break;
        case(1):
            // thinkAleatorio(c_piece, id_piece, dice);
            // MiniMax(*actual, id, 0, PROFUNDIDAD_MINIMAX, c_piece, id_piece, dice, ValoracionTest);
            PodaAlfaBeta(*actual, id, 0, PROFUNDIDAD_ALFABETA, c_piece, id_piece, dice, menosinf, masinf, ValoracionTest);
            break;
        case(2):
            thinkFichaMasAdelantada(c_piece, id_piece, dice);
            break;
        case(3):
            thinkMejorOpcion(c_piece, id_piece, dice);
            break;
    }
}

bool AIPlayer::move(){
    cout << "Realizo un movimiento automatico" << endl;
    
    color c_piece;
    int id_piece;
    int dice;
    think(c_piece, id_piece, dice);

    cout << "Movimiento elegido: " << str(c_piece) << " " << id_piece << " " << dice << endl;

    actual->movePiece(c_piece, id_piece, dice);
    return true;
}

void AIPlayer::thinkAleatorio(color &c_piece, int &id_piece, int &dice) const {

    // El color de ficha que se va a mover
    c_piece = actual->getCurrentColor();

    // Vector que almacenará los dados que se pueden usar para el movimiento
    vector<int> current_dices;
    // Vector que almacenará los ids de las fichas que se pueden mover para el dado elegido.
    vector<int> current_pieces;

    // Se obtiene el vector de dados que se pueden usar para el movimiento
    current_dices = actual->getAvailableDices(c_piece);
    // Elijo un dado de forma aleatoria.
    dice = current_dices[rand() % current_dices.size()];

    // Se obtiene el vector de fichas que se pueden mover para el dado elegido
    current_pieces = actual->getAvailablePieces(c_piece, dice);

    // Si tengo fichas para el dado elegido muevo una al azar.
    if(current_pieces.size() > 0){
        id_piece = current_pieces[rand() % current_pieces.size()];
    }
    else{
        // Si no tengo fichas para el dado elegido, pasa turno (la macro SKIP_TURN me permite no mover).
        id_piece = SKIP_TURN;
    }
}

void AIPlayer::thinkAleatorioMasInteligente(color &c_piece, int &id_piece, int &dice) const {
    // El color de ficha que se va a mover
    c_piece = actual->getCurrentColor();

    // Vector que almacenará los dados que se pueden usar para el movimiento
    vector<int> current_dices;

    // Vector que almacenará los ids de las fichas que se pueden mover para el dado elegido.
    vector<int> current_pieces;

    // Se obtiene el vector de dados que se pueden usar para el movimiento
    current_dices = actual->getAvailableDices(c_piece);

    // En vez de elegir un dado al azar, miro primero cuáles tienen fichas que se puedan mover.
    vector<int> current_dices_que_pueden_mover_ficha;
    for(int i=0; i<current_dices.size(); ++i) {
        // Se obtiene el vector de fichas que se pueden mover para el dado elegido.
        current_pieces = actual->getAvailablePieces(c_piece, current_dices[i]);

        // Si se pueden mover fichas para el dado actual, lo añado al vector de dados que pueden mover fichas.
        if(current_pieces.size()>0) {
            current_dices_que_pueden_mover_ficha.push_back(current_dices[i]);
        }
    }

    // Si no tengo ningún dado que pueda mover fichas, paso turno con un dado al azar (SKIP_TURN)
    // En caso contrario, elijo un dado de forma aleatoria de entre los que pueden mover ficha.
    if(current_dices_que_pueden_mover_ficha.size()==0) {
        dice = current_dices[rand()%current_dices.size()];
        id_piece = SKIP_TURN;
    } else {
        dice = current_dices_que_pueden_mover_ficha[rand()%current_dices_que_pueden_mover_ficha.size()];

        // Se obtiene el vector de fichas que se pueden mover para el dado elegido.
        current_pieces = actual->getAvailablePieces(c_piece, dice);

        // Muevo una ficha al azar de entre las que se pueden mover.
        id_piece = current_pieces[rand()%current_pieces.size()];
    }
}

void AIPlayer::thinkFichaMasAdelantada(color &c_piece, int &id_piece, int &dice) const {
    // Elijo el dado haciendo lo mismo que el jugador anterior
    thinkAleatorioMasInteligente(c_piece,  id_piece, dice);
    // Tras llamar a esta funcion, ya tengo en dice el número de dado que quiero usar.
    // Ahora, en vez de mover una ficha al azar, voy a mover la que esté más adelantada
    // (equivalentemente, la más cercana a la meta).

    vector<int> current_pieces = actual->getAvailablePieces(c_piece, dice);

    int id_ficha_mas_adelantada = -1;
    int min_distancia_meta = 9999;
    for(int i=0; i<current_pieces.size(); ++i) {
        // distanceToGoal(color,id) devuelve la distancia a la meta de la ficha [id] del color que le indique.
        int distancia_meta = actual->distanceToGoal(c_piece, current_pieces[i]);
        if(distancia_meta<min_distancia_meta) {
            min_distancia_meta = distancia_meta;
            id_ficha_mas_adelantada = current_pieces[i];
        }
    }

    // Si no he encontrado ficha, paso turno.
    // En caso contrario, moveré la ficha más adelantada.
    if(id_ficha_mas_adelantada==-1) {
        id_piece = SKIP_TURN;
    } else {
        id_piece = id_ficha_mas_adelantada;
    }
}

void AIPlayer::thinkMejorOpcion(color &c_piece, int &id_piece, int &dice) const {
    // Vamos a mirar todos los posibles movimientos del jugador actual accediendo a los hijos del estado actual.

    // generateNextMove va iterando sobre cada hijo. Le paso la accion del ultimo movimiento sobre
    // el que he iterado y me devolverá el siguiente. Inicialmente, cuando aún no he hecho ningún
    // movimiento, se debe inicializar de la siguiente manera: (ver tutorial, página 13)
    color last_c_piece = none;  // El color de la última ficha que se movió.
    int last_id_piece = -1;     // El id de la última ficha que se movió.
    int last_dice = -1;         // El dado que se usó en el último movimiento.

    // Cuando ya he recorrido todos los hijos, la función devuelve el estado actual. De esta forma puedo saber
    // cuándo paro de iterar.

    Parchis siguiente_hijo = actual->generateNextMove(last_c_piece, last_id_piece, last_dice);

    bool me_quedo_con_esta_accion = false;

    while( !(siguiente_hijo == *actual) && !me_quedo_con_esta_accion ) {
        // Si con este movimiento como ficha, o llego a la meta, o gano la partida
        // En caso contrario, genero el siguiente hijo y vuelvo a comprobar.
        if( siguiente_hijo.isEatingMove() || siguiente_hijo.isGoalMove() ||
            (siguiente_hijo.gameOver() && siguiente_hijo.getWinner()==this->jugador)) {
            // Me quedo con la acción actual (se almacenó en last_c_piece, last_id_piece, last_dice al llamar a generateNextMove).
            me_quedo_con_esta_accion = true;
        } else {
            // Genero el siguiente hijo
            siguiente_hijo = actual->generateNextMove(last_c_piece, last_id_piece, last_dice);
        }
    }

    // Si he encontrado una acción que me interesa, la guardo en las variables pasadas por referencia
    // Si no, muevo la ficha más adelantada como antes.
    if(me_quedo_con_esta_accion) {
        c_piece = last_c_piece;
        id_piece = last_id_piece;
        dice = last_dice;
    } else {
        thinkFichaMasAdelantada(c_piece, id_piece, dice);
    }
}

// Algoritmo MiniMax
double AIPlayer::MiniMax(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, double (*heuristic)(const Parchis &, int)) const {

    if(actual.gameOver() || profundidad==profundidad_max) {
        return heuristic(actual, jugador);
    }

    // Guardan la accion a realizar para llegar al k-ésimo hijo
    color last_c_piece = none;  // El color de la última ficha que se movió.
    int last_id_piece = -1;     // El id de la última ficha que se movió.
    int last_dice = -1;         // El dado que se usó en el último movimiento.

    // Auxiliares, son variables estáticas (sólo se declaran una vez, más eficiente)
    color aux_c_piece;
    int aux_id_piece;
    int aux_dice;

    Parchis siguiente_hijo = actual.generateNextMove(last_c_piece, last_id_piece, last_dice);
    double minimax = MiniMax(siguiente_hijo, jugador, profundidad + 1,
                             profundidad_max, aux_c_piece, aux_id_piece, aux_dice, heuristic);

    c_piece = last_c_piece;
    id_piece = last_id_piece;
    dice = last_dice;

    double aux = minimax;

    // Comparo con todos los hijos hasta obtener el mejor.
    while (!(siguiente_hijo == actual)) {
        if (jugador == actual.getCurrentPlayerId()) {
            // Nodo MAX: busca su máximo hijo.
            // Si el valor del hijo es mayor que el máximo actual, lo actualizo.
            if (aux > minimax) {
                minimax = aux;
                c_piece = last_c_piece;
                id_piece = last_id_piece;
                dice = last_dice;
            }
        } else {
            // Nodo MIN: busca su mínimo hijo.
            // Si el valor del hijo es menor que el mínimo actual, lo actualizo.
            if (aux < minimax) {
                minimax = aux;
                c_piece = last_c_piece;
                id_piece = last_id_piece;
                dice = last_dice;
            }
        }

        siguiente_hijo = actual.generateNextMove(last_c_piece, last_id_piece, last_dice);
        // Calcula el valor del hijo k-ésimo
        if(!(siguiente_hijo==actual)) {
            aux = MiniMax(siguiente_hijo, jugador, profundidad + 1,
                          profundidad_max, aux_c_piece, aux_id_piece, aux_dice, heuristic);
        }
    }

    // Devolver V(J)
    return minimax;
}

// Algoritmo Poda Alfa Beta
double AIPlayer::PodaAlfaBeta(const Parchis &actual, int jugador, int profundidad, int profundidad_max, color &c_piece, int &id_piece, int &dice, double alpha, double beta, double (*heuristic)(const Parchis &, int)) const {
    if(actual.gameOver() || profundidad==profundidad_max) {
        return heuristic(actual, jugador);
    }

    // Guardan la accion a realizar para llegar al k-ésimo hijo
    color last_c_piece = none;  // El color de la última ficha que se movió.
    int last_id_piece = -1;     // El id de la última ficha que se movió.
    int last_dice = -1;         // El dado que se usó en el último movimiento.

    // Auxiliares, son variables estáticas (sólo se declaran una vez, más eficiente)
    double aux = 0;
    color aux_c_piece;
    int aux_id_piece;
    int aux_dice;

    Parchis siguiente_hijo = actual.generateNextMoveDescending(last_c_piece, last_id_piece, last_dice);
    if (jugador == actual.getCurrentPlayerId()) {   // Nodo MAX
        while(!(siguiente_hijo == actual)) {
            aux = PodaAlfaBeta (siguiente_hijo, jugador, profundidad + 1,
                                profundidad_max, c_piece, id_piece,
                                dice, alpha, beta, heuristic);

            if (aux > alpha) {
                alpha = aux;
                aux_c_piece = last_c_piece;
                aux_id_piece = last_id_piece;
                aux_dice = last_dice;
            }

            if(alpha>=beta) {
                return beta;
            }

            siguiente_hijo = actual.generateNextMoveDescending(last_c_piece, last_id_piece, last_dice);
        }

        if (profundidad==0) {
            c_piece = aux_c_piece;
            id_piece = aux_id_piece;
            dice = aux_dice;
        }

        return alpha;

    } else {    // Nodo MIN
        while(!(siguiente_hijo == actual)) {
            aux = PodaAlfaBeta (siguiente_hijo, jugador, profundidad + 1,
                                profundidad_max, c_piece, id_piece,
                                dice, alpha, beta, heuristic);

            if (aux < beta) {
                beta = aux;
                aux_c_piece = last_c_piece;
                aux_id_piece = last_id_piece;
                aux_dice = last_dice;
            }

            if(alpha>=beta) {
                return alpha;
            }

            siguiente_hijo = actual.generateNextMoveDescending(last_c_piece, last_id_piece, last_dice);
        }

        if (profundidad==0) {
            c_piece = aux_c_piece;
            id_piece = aux_id_piece;
            dice = aux_dice;
        }

        return beta;
    }
}

// Heuristicas

// Heurística de prueba proporcionada para validar el funcionamiento del algoritmo de búsqueda.
double AIPlayer::ValoracionTest(const Parchis &estado, int jugador) {

    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador)
    {
        return gana;
    }
    else if (ganador == oponente)
    {
        return pierde;
    }
    else
    {
        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int puntuacion_jugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < my_colors.size(); i++)
        {
            color c = my_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                // Valoro positivamente que la ficha esté en casilla segura o meta.
                if (estado.isSafePiece(c, j))
                {
                    puntuacion_jugador++;
                }
                else if (estado.getBoard().getPiece(c, j).type == goal)
                {
                    puntuacion_jugador += 5;
                }
            }
        }

        // Recorro todas las fichas del oponente
        int puntuacion_oponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < op_colors.size(); i++)
        {
            color c = op_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++)
            {
                if (estado.isSafePiece(c, j))
                {
                    // Valoro negativamente que la ficha esté en casilla segura o meta.
                    puntuacion_oponente++;
                }
                else if (estado.getBoard().getPiece(c, j).type == goal)
                {
                    puntuacion_oponente += 5;
                }
            }
        }

        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return puntuacion_jugador - puntuacion_oponente;
    }
}

double AIPlayer::myHeuristic1(const Parchis &estado, int jugador) {

    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {
        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        int puntuacion_jugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < my_colors.size(); i++) {
            color c = my_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                if (estado.isWall(estado.getBoard().getPiece(c, j)) == c) {
                    // Debe ser una mitad, puesto que si está en una barrera,
                    // la puntuación se incrementará dos veces.
                    puntuacion_jugador += 0.5;
                } else if (estado.isSafePiece(c, j)) {
                    puntuacion_jugador ++;
                } else if (estado.getBoard().getPiece(c, j).type == goal) {
                    puntuacion_jugador += 5;
                } else if (estado.getBoard().getPiece(c, j).type == final_queue) {
                    puntuacion_jugador += 2;
                } else if (estado.getBoard().getPiece(c, j).type == home) {
                    puntuacion_jugador -= 10;
                }
            }
            if (estado.isEatingMove() && estado.getCurrentColor() == c) {  // Nuestro color ha comido pieza
                if (estado.eatenPiece().first != my_colors[(i + 1) % 2]) {
                    puntuacion_jugador += 10;
                }
            } else if (estado.isGoalMove() && estado.getCurrentColor() == c) {
                puntuacion_jugador += 2;    // ¿Necesario? Ya se tiene en cuenta en goal
            }
        }

        // Recorro todas las fichas del oponente
        int puntuacion_oponente = 0;
        for (int i = 0; i < my_colors.size(); i++) {
            color c = op_colors[i];
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                if (estado.isWall(estado.getBoard().getPiece(c, j)) == c) {
                    puntuacion_oponente += 0.5;
                } else if (estado.isSafePiece(c, j)) {
                    puntuacion_oponente ++;
                } else if (estado.getBoard().getPiece(c, j).type == goal) {
                    puntuacion_oponente += 5;
                } else if (estado.getBoard().getPiece(c, j).type == final_queue) {
                    puntuacion_oponente += 2;
                } else if (estado.getBoard().getPiece(c, j).type == home) {
                    puntuacion_oponente -= 10;
                }
            }
            if (estado.isEatingMove() && estado.getCurrentColor() == c) {
                if (estado.eatenPiece().first != op_colors[(i + 1) % 2]) {
                    puntuacion_oponente += 10;
                }
            } else if (estado.isGoalMove() && estado.getCurrentColor() == c) {
                puntuacion_oponente += 2;    // ¿Necesario? Ya se tiene en cuenta en goal
            }
        }
        // Devuelvo la puntuación de mi jugador menos la puntuación del oponente.
        return puntuacion_jugador - puntuacion_oponente;
    }
}

// La mejor heurística, tiene en cuenta la distancia a la meta. No termina de funcionar (valores muy grandes) ¿por qué?
double AIPlayer::myHeuristic2(const Parchis &estado, int jugador) {
    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {

        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        // Recorro todas las fichas de mi jugador
        double puntuacion_jugador = 0;
        // Recorro colores de mi jugador.
        for (int i = 0; i < my_colors.size(); i++) {
            color c = my_colors[i];
            double puntuacion_color = 0;
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                puntuacion_color = 74 - estado.distanceToGoal(c, j) - 5;

                if (estado.getBoard().getPiece(c, j).type == goal) {
                    puntuacion_color += 250;
                } else if (estado.getBoard().getPiece(c, j).type == final_queue) {
                    puntuacion_color += 30;
                } else if (estado.getBoard().getPiece(c, j).type == home) {
                    puntuacion_color -= 500;
                }

                if (estado.isWall(estado.getBoard().getPiece(c, j)) == c) {  // Si está en una barrera
                    puntuacion_color *= 1.15;
                }

                if (estado.isSafePiece(c, j)) {
                    puntuacion_color *= 1.75;
                }
            }
            if (estado.isEatingMove() && estado.getCurrentColor() == c &&
                estado.eatenPiece().first != my_colors[(i + 1) % 2]) {
                puntuacion_color += 1000;
            }
            puntuacion_jugador += puntuacion_color;
        }

        // Recorro todas las fichas del oponente
        double puntuacion_oponente = 0;
        // Recorro colores del oponente.
        for (int i = 0; i < op_colors.size(); i++) {
            color c = op_colors[i];
            double puntuacion_color = 0;
            // Recorro las fichas de ese color.
            for (int j = 0; j < num_pieces; j++) {
                puntuacion_color = 74 - estado.distanceToGoal(c, j) - 5;

                if (estado.getBoard().getPiece(c, j).type == goal) {
                    puntuacion_color += 250;
                } else if (estado.getBoard().getPiece(c, j).type == final_queue) {
                    puntuacion_color += 30;
                } else if (estado.getBoard().getPiece(c, j).type == home) {
                    puntuacion_color -= 800;
                }

                if (estado.isWall(estado.getBoard().getPiece(c, j)) == c) {  // Si está en una barrera
                    puntuacion_color *= 1.15;
                }

                if (estado.isSafePiece(c, j)) {
                    puntuacion_color *= 1.75;
                }
            }

            if (estado.isEatingMove() && estado.getCurrentColor() == c) {
                puntuacion_color += 400;
            }
            puntuacion_oponente += puntuacion_color;
        }
        return puntuacion_jugador - puntuacion_oponente;
    }
}

// Métodos para la heuristica 3, demasiado compleja y no funciona correctamente

// Comprueba si {c,idx} puede llegar a la meta
bool AIPlayer::canReachGoal(const Parchis &estado, const color &c, int idx) {
    return estado.isLegalMove(c,estado.getBoard().getPiece(c,idx),estado.distanceToGoal(c,idx));
}

// Comprueba si {c1,id1} puede comerse a {c2,id2}
bool AIPlayer::canEat(const Parchis &estado, const color c1, int id1, const color c2, int id2) {
    // Si son del mismo color, hay barreras entre ellas o {c2,id2} está en una casilla segura, no podrá comersela
    // En el caso de que c2 esté en una barrera, esto se detecta en el método isLegalMove
    if(c1==c2 || estado.isSafePiece(c2,id2)) {
        return false;
    } else {
        int distance = estado.distanceBoxtoBox(c1,id1,c2,id2);
        if(estado.isLegalMove(c1,estado.getBoard().getPiece(c1, id1),distance)) {
            return true;
        }
        // Si se ha llegado hasta aquí, es porque no hay ningún dado que le permita comérsela
        return false;
    }
}

double AIPlayer::getMeanDistanceToGoal(const Parchis &estado, const color c) {
    int sum = 0;
    for(int i=0; i<num_pieces; ++i)
        sum += estado.distanceToGoal(c,i);

    double mean = (double)sum/num_pieces;
    return mean;
}

int AIPlayer::getIdPieceMasAdelantada(const Parchis &estado, const color c) {
    int min_distance = 1000;
    int id_min;
    for(int i=0; i<num_pieces; ++i) {
        int d = estado.distanceToGoal(c,i);
        if(d>0 && d<min_distance) {
            min_distance = d;
            id_min = i;
        }
    }
    return id_min;
}

double AIPlayer::ranking(const Parchis &estado, int jugador, color c_piece, int id_piece) {
    double ranking = 0;

    // Colores que juega mi jugador y colores del oponente
    int oponente = (jugador + 1) % 2;
    vector<color> my_colors = estado.getPlayerColors(jugador);
    vector<color> op_colors = estado.getPlayerColors(oponente);

    // Según su posicion y la posicion de los oponentes
    Box box = estado.getBoard().getPiece(c_piece, id_piece);
    const vector<pair <color, int>> occupation = estado.boxState(box);

    switch(box.type) {
        case(goal):
            ranking += 150;
            break;
        case(final_queue):
            ranking += 20;
            // Si tiene en los dados disponibles, la distancia para llegar a la meta, aumenta su ranking
            if(canReachGoal(estado, c_piece, id_piece)) {
                ranking += 30;
            }
            break;
        case(home):
            ranking -= 15;
            // Si puede salir de la casa, su ranking mejora ligeramente
            if(estado.isLegalMove(c_piece, box, 5)) {
                ranking += 5;
            }
            break;
        case(normal):
            // (A) Según su posición en el tablero y con respecto al resto de piezas

            // Se encuentra formando una barrera
            if(occupation.size()==2) {
                if(estado.isWall(box)==c_piece) { // La barrera es del mismo color
                    ranking += 10;
                } else if ( (occupation[0].first==my_colors[0] || occupation[0].first==my_colors[1]) &&
                            (occupation[1].first==my_colors[0] || occupation[1].first==my_colors[1]) ) {    // La barrera es con su otro color
                    ranking += 5;
                } else {    // La barrera es con su oponente
                    ranking -= 5;
                }
            }

            // Está en una casilla segura, no es opuesto al caso de la barrera, pueden ocurrir ambas cosas
            if(estado.isSafePiece(c_piece, id_piece)) {
                ranking += 20;
            } else if (occupation.size()<2) { // Esta en una casilla normal, no segura (puede ser comida)
                // Comprueba si puede ser comida por las piezas propias de otro color.
                for (int i = 0; i<my_colors.size(); ++i) {
                    if(my_colors[i]!=c_piece) {
                        for(int j=0; j<num_pieces; ++j) {   // Puede ser comida por otra ficha propia de otro color
                            if(canEat(estado, my_colors[i], j, c_piece, id_piece))
                                ranking -= 10;
                        }
                    }
                }
                // Comprueba si puede ser comida por las piezas del oponente
                for (int i = 0; i<op_colors.size(); ++i) {
                    for(int j=0; j<num_pieces; ++j) {   // Puede ser comida por otra ficha propia de otro color
                        if(canEat(estado, op_colors[i], j, c_piece, id_piece))
                            ranking -= 50;
                    }
                }
            }

            // (B) Según los dados disponibles y sus movimientos

            // Comprueba si puede llegar a la meta
            if(canReachGoal(estado, c_piece, id_piece)) {
                ranking += 50;
            } else {  // Tener en cuenta la distancia de la ficha hasta la meta
                double n = 20*1.0/estado.distanceToGoal(c_piece,id_piece);
                ranking += n;
            }

            // Comprueba si puede comerse a piezas de su otro color (puede llegar a beneficiar, pero por lo
            // general lo evitará)
            for (int i = 0; i<my_colors.size(); ++i) {
                if(my_colors[i]!=c_piece) {
                    for(int j=0; j<num_pieces; ++j) {
                        if(canEat(estado, c_piece, id_piece, my_colors[i], j))
                            ranking += 5;
                    }
                }
            }

            // Comprueba si puede comerse a piezas de su oponente
            for (int i = 0; i<op_colors.size(); ++i) {
                for(int j=0; j<num_pieces; ++j) {   // Puede ser comida por otra ficha propia de otro color
                    if(canEat(estado, c_piece, id_piece, op_colors[i], j))
                        ranking += 50;
                }
            }

            // Comprueba si puede formar barrera con alguna otra pieza de su color (siempre y cuando no sea
            // en el pasillo final!!)                ¿?

            break;
    }

    // Finalmente comprueba si es la pieza más adelantada, puesto que tiene un potenciador ( es la más importante )
    if(getIdPieceMasAdelantada(estado, c_piece) == id_piece)
        ranking *= 1.5;

    return ranking;
}

// Heuristica 3, ranking de las piezas
double AIPlayer::myHeuristic3(const Parchis &estado, int jugador) {

    int ganador = estado.getWinner();
    int oponente = (jugador + 1) % 2;

    // Si hay un ganador, devuelvo más/menos infinito, según si he ganado yo o el oponente.
    if (ganador == jugador) {
        return gana;
    } else if (ganador == oponente) {
        return pierde;
    } else {
        // Recorro todas mis fichas y las de mi oponente y les calculo su ranking

        // Colores que juega mi jugador y colores del oponente
        vector<color> my_colors = estado.getPlayerColors(jugador);
        vector<color> op_colors = estado.getPlayerColors(oponente);

        double puntuacion_jugador = 0;
        for (int i = 0; i < my_colors.size(); ++i) {
            for (int j = 0; j < num_pieces; ++j) {
                double r = ranking(estado, jugador, my_colors[i], j);
                puntuacion_jugador += r;
            }
        }

        double puntuacion_oponente = 0;
        for (int i = 0; i < op_colors.size(); ++i) {
            for (int j = 0; j < num_pieces; ++j) {
                double r = ranking(estado, oponente, op_colors[i], j);
                puntuacion_oponente += r;
            }
        }

        return (puntuacion_jugador - puntuacion_oponente);
    }
}
