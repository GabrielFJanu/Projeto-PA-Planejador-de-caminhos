#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "planejador.h"

using namespace std;

/* *************************
   * CLASSE IDPONTO        *
   ************************* */

/// Atribuicao de string
void IDPonto::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE IDROTA         *
   ************************* */

/// Atribuicao de string
void IDRota::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE PONTO          *
   ************************* */

/// Distancia entre 2 pontos (formula de haversine)
double Ponto::distancia(const Ponto& P) const
{
  // Gera excecao se pontos invalidos
  if (!valid() || !P.valid())
    throw invalid_argument("distancia: ponto(s) invalido(s)");

  // Tratar logo pontos identicos
  if (id == P.id) return 0.0;
  // Constantes
  static const double MY_PI = 3.14159265358979323846;
  static const double R_EARTH = 6371.0;
  // Conversao para radianos
  double lat1 = MY_PI*this->latitude/180.0;
  double lat2 = MY_PI*P.latitude/180.0;
  double lon1 = MY_PI*this->longitude/180.0;
  double lon2 = MY_PI*P.longitude/180.0;
  // Seno das diferencas
  double sin_dlat2 = sin((lat2-lat1)/2.0);
  double sin_dlon2 = sin((lon2-lon1)/2.0);
  // Quadrado do seno do angulo entre os pontos
  double a = sin_dlat2*sin_dlat2 + cos(lat1)*cos(lat2)*sin_dlon2*sin_dlon2;
  // Em vez de utilizar a funcao arcosseno, asin(sqrt(sin2_ang)),
  // vou utilizar a funcao arcotangente, menos sensivel a erros numericos.
  // Distancia entre os pontos
  return 2.0*R_EARTH*atan2(sqrt(a),sqrt(1-a));
}

/* *************************
   * CLASSE ROTA           *
   ************************* */

/// Retorna a outra extremidade da rota, a que nao eh o parametro.
/// Gera excecao se o parametro nao for uma das extremidades da rota.
IDPonto Rota::outraExtremidade(const IDPonto& ID) const
{
  if (extremidade[0]==ID) return extremidade[1];
  if (extremidade[1]==ID) return extremidade[0];
  throw invalid_argument("outraExtremidade: invalid IDPonto parameter");
}

/* *************************
   * CLASSE PLANEJADOR     *
   ************************* */

/// Torna o mapa vazio
void Planejador::clear()
{
  pontos.clear();
  rotas.clear();
}

/// Retorna um Ponto do mapa, passando a id como parametro.
/// Se a id for inexistente, gera excecao.
Ponto Planejador::getPonto(const IDPonto& Id) const
{
  // Procura um ponto que corresponde aa Id do parametro
  auto itr = find(pontos.begin(),pontos.end(),Id); //CLASSE PONTO PRECISA DO OPERATOR==(IDPONTO)
  // Em caso de sucesso, retorna o ponto encontrado
  if(itr!=pontos.end()){return *itr;}
  // Se nao encontrou, gera excecao
  throw invalid_argument("getPonto: invalid IDPonto parameter");
}

/// Retorna um Rota do mapa, passando a id como parametro.
/// Se a id for inexistente, gera excecao.
Rota Planejador::getRota(const IDRota& Id) const
{
  // Procura uma rota que corresponde aa Id do parametro
  auto itr = find(rotas.begin(),rotas.end(),Id); //CLASSE ROTA PRECISA DO OPERATOR==(IDROTA)
  // Em caso de sucesso, retorna a rota encontrada
  if(itr!=rotas.end()){return *itr;}
  // Se nao encontrou, gera excecao
  throw invalid_argument("getRota: invalid IDRota parameter");
}

/// Imprime os pontos do mapa no console
void Planejador::imprimirPontos() const
{
  for (const auto& P : pontos)
  {
    cout << P.id << '\t' << P.nome
         << " (" <<P.latitude << ',' << P.longitude << ")\n";
  }
}

/// Imprime as rotas do mapa no console
void Planejador::imprimirRotas() const
{
  for (const auto& R : rotas)
  {
    cout << R.id << '\t' << R.nome << '\t' << R.comprimento << "km"
         << " [" << R.extremidade[0] << ',' << R.extremidade[1] << "]\n";
  }
}

/// Leh um mapa dos arquivos arq_pontos e arq_rotas.
/// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e gera excecao ios_base::failure.
void Planejador::ler(const std::string& arq_pontos,
                     const std::string& arq_rotas)
{
  // Listas temporarias para armazenamento dos Pontos e Rotas lidos.
  list<Ponto> temp_pontos;
  list<Rota> temp_rotas;
  //auxiliares
  string prov;
  Ponto P;
  Rota R;
  // Leh os Pontos do arquivo e armazena na lista temporaria de Pontos.
  // 1) Abre o arquivo de Pontos (Em caso de erro, codigo 1)
  ifstream arqp(arq_pontos);
  if(!arqp.is_open()){throw ios_base::failure("Erro 1 na leitura do arquivo de pontos "+arq_pontos);}
  // 2) Leh e testa o cabecalho do arquivo: "ID;Nome;Latitude;Longitude"
  //    (Em caso de erro, codigo 2)
  getline(arqp, prov);
  if(prov!="ID;Nome;Latitude;Longitude"){throw ios_base::failure("Erro 2 na leitura do arquivo de pontos "+arq_pontos);}
  // 3) Leh os Pontos, um em cada linha, ateh que o arquivo acabe:
  while(!arqp.eof()){
    //    3.1) Leh a ID (Em caso de erro, codigo 3)
    getline(arqp, prov,';');
    if(arqp.fail()){throw ios_base::failure("Erro 3 na leitura do arquivo de pontos "+arq_pontos);}
    P.id.set(move(prov));
    //    3.2) Leh o nome (Em caso de erro, codigo 4)
    getline(arqp, P.nome,';');
    if(arqp.fail()){throw ios_base::failure("Erro 4 na leitura do arquivo de pontos "+arq_pontos);}
    //    3.3) Leh a latitude (Em caso de erro, codigo 5)
    arqp >> P.latitude;
    if(arqp.fail()){throw ios_base::failure("Erro 5 na leitura do arquivo de pontos "+arq_pontos);}
    arqp.ignore(1,';');
    //    3.4) Leh a longitude (Em caso de erro, codigo 6)
    arqp >> P.longitude;
    if(arqp.fail()){throw ios_base::failure("Erro 6 na leitura do arquivo de pontos "+arq_pontos);}
    arqp >> ws;
    //    3.5) Confirma que o Ponto com esses parametros lidos eh valido
    //         (Em caso de erro, codigo 7)
    if(!P.valid()){throw ios_base::failure("Erro 7 na leitura do arquivo de pontos "+arq_pontos);}
    //    3.6) Confirma que nao existe Ponto com a mesma ID na lista temporaria
    //         de Pontos (Em caso de erro, codigo 8)
    if(any_of(temp_pontos.begin(),temp_pontos.end(),[&P](const Ponto& P2){return P==P2;})){throw ios_base::failure("Erro 8 na leitura do arquivo de pontos "+arq_pontos);} //CLASSE PONTO PRECISA DO OPERATOR==(PONTO)
    //    3.7) Insere o Ponto lido na lista temporaria de Pontos
    temp_pontos.push_back(P);
  }
  //    Em caso de qualquer erro, gera excecao ios_base::failure com mensagem:
  //      "Erro <CODIGO> na leitura do arquivo de pontos <ARQ_PONTOS>"
  // 4) Fecha o arquivo de Pontos
  arqp.close();

  // Leh as Rotas do arquivo e armazena na lista temporaria de Rotas.
  // 1) Abre o arquivo de Rotas (Em caso de erro, codigo 1)
  ifstream arqr(arq_rotas);
  if(!arqr.is_open()){throw ios_base::failure("Erro 1 na leitura do arquivo de rotas "+arq_rotas);}
  // 2) Leh e testa o cabecalho do arquivo: "ID;Nome;Extremidade 1;Extremidade 2;Comprimento"
  //    (Em caso de erro, codigo 2)
  getline(arqr, prov);
  if(prov!="ID;Nome;Extremidade 1;Extremidade 2;Comprimento"){throw ios_base::failure("Erro 2 na leitura do arquivo de rotas "+arq_rotas);}
  // 3) Leh as Rotas, uma em cada linha, ateh que o arquivo acabe:
  while(!arqr.eof()){
    //    3.1) Leh a ID (Em caso de erro, codigo 3)
    getline(arqr, prov,';');
    if(arqr.fail()){throw ios_base::failure("Erro 3 na leitura do arquivo de rotas "+arq_rotas);}
    R.id.set(move(prov));
    //    3.2) Leh o nome (Em caso de erro, codigo 4)
    getline(arqr, R.nome,';');
    if(arqr.fail()){throw ios_base::failure("Erro 4 na leitura do arquivo de rotas "+arq_rotas);}
    //    3.3) Leh a ID da extremidade[0] (Em caso de erro, codigo 5)
    getline(arqr, prov,';');
    if(arqr.fail()){throw ios_base::failure("Erro 5 na leitura do arquivo de rotas "+arq_rotas);}
    R.extremidade[0].set(move(prov));
    //    3.4) Leh a ID da extremidade[1] (Em caso de erro, codigo 6)
    getline(arqr, prov,';');
    if(arqr.fail()){throw ios_base::failure("Erro 6 na leitura do arquivo de rotas "+arq_rotas);}
    R.extremidade[1].set(move(prov));
    //    3.5) Leh o comprimento (Em caso de erro, codigo 7)
    arqr >> R.comprimento;
    if(arqr.fail()){throw ios_base::failure("Erro 7 na leitura do arquivo de rotas "+arq_rotas);}
    arqr >> ws;
    //    3.6) Confirma que a Rota com esses parametros lidos eh valida
    //         (Em caso de erro, codigo 8)
    if(!R.valid()){throw ios_base::failure("Erro 8 na leitura do arquivo de rotas "+arq_rotas);}
    //    3.7) Confirma que a Id da extremidade[0] corresponde a um ponto lido
    //         na lista temporaria de Pontos (Em caso de erro, codigo 9)
    if(none_of(temp_pontos.begin(),temp_pontos.end(),[&R](const Ponto& P2){return P2==R.extremidade[0];})){throw ios_base::failure("Erro 9 na leitura do arquivo de rotas "+arq_rotas);} //CLASSE PONTO PRECISA DO OPERATOR==(IDPONTO)
    //    3.8) Confirma que a Id da extremidade[1] corresponde a um ponto lido
    //         na lista temporaria de Pontos (Em caso de erro, codigo 10)
    if(none_of(temp_pontos.begin(),temp_pontos.end(),[&R](const Ponto& P2){return P2==R.extremidade[1];})){throw ios_base::failure("Erro 10 na leitura do arquivo de rotas "+arq_rotas);} //CLASSE PONTO PRECISA DO OPERATOR==(IDPONTO)
    //    3.9) Confirma que nao existe Rota com a mesma ID na lista temporaria
    //         de Rotas (Em caso de erro, codigo 11)
    if(any_of(temp_rotas.begin(),temp_rotas.end(),[&R](const Rota& R2){return R==R2;})){throw ios_base::failure("Erro 11 na leitura do arquivo de rotas "+arq_rotas);} //CLASSE ROTA PRECISA DO OPERATOR==(ROTA)
    //    3.10) Insere a Rota lida na lista temporaria de Rotas
    temp_rotas.push_back(R);
  }
  //    Em caso de qualquer erro, gera excecao ios_base::failure com mensagem:
  //      "Erro <CODIGO> na leitura do arquivo de rotas <ARQ_ROTAS>"
  // 4) Fecha o arquivo de Rotas
  arqr.close();

  // Faz as listas temporarias de Pontos e Rotas passarem a ser
  // as listas de Pontos e Rotas do planejador.
  pontos = move(temp_pontos);
  rotas = move(temp_rotas);
}

/// *******************************************************************************
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// *******************************************************************************

/// Noh: os elementos dos conjuntos de busca do algoritmo A*

struct Noh{
    //id do ponto
    IDPonto id_pt;
    //id da rota do antecessor ateh o ponto
    IDRota id_rt;
    //custo passado
    double g;
    //custo futuro
    double h;
    //custo total
    double f() const{return g+h;};
    //Construtor default
    Noh(): id_pt(), id_rt(), g(0.0), h(0.0) {}
    //sobrecarga do operator==(IDPONTO)
    bool operator==(const IDPonto& ID) const{
        return this->id_pt==ID;
    }
    bool operator<(const Noh& N) const{
        return this->f()<N.f();
    }
};

/// Calcula o caminho mais curto no mapa entre origem e destino, usando o algoritmo A*
/// Retorna o comprimento do caminho encontrado (<0 se nao existe caminho).
/// O parametro C retorna o caminho encontrado (vazio se nao existe caminho).
/// O parametro NA retorna o numero de nos (>=0) em Aberto ao termino do algoritmo A*,
/// mesmo quando nao existe caminho.
/// O parametro NF retorna o numero de nos (>=0) em Fechado ao termino do algoritmo A*,
/// mesmo quando nao existe caminho.
/// Em caso de parametros de entrada invalidos ou de erro no algoritmo, gera excecao.
double Planejador::calculaCaminho(const IDPonto& id_origem,
                                  const IDPonto& id_destino,
                                  Caminho& C, int& NA, int& NF)
{
  // Zera o caminho resultado
  C.clear();

  try
  {
    // Mapa vazio
    if (empty()) throw 1;

    Ponto pt_orig, pt_dest;
    // Calcula os pontos que correspondem a id_origem e id_destino.
    // Se algum nao existir, throw 2
    try
    {
      pt_orig = getPonto(id_origem);
      pt_dest = getPonto(id_destino);
    }
    catch(...)
    {
      throw 2;
    }

    /* *****************************  /
    /  IMPLEMENTACAO DO ALGORITMO A*  /
    /  ***************************** */
    //atual <- Noh inicial
    Noh atual;
    atual.id_pt = id_origem;
    try
    {
      atual.h = pt_orig.distancia(pt_dest);
    }
    catch(...)
    {
      throw 3;
    }
    //inicializa os conjuntos de Nohs
    list<Noh> Aberto;
    list<Noh> Fechado;
    Aberto.push_back(atual);
    //auxiliares
    list<Rota>::iterator itr_rotas;
    Rota rota_suc, rota_ant;
    Noh suc;
    Ponto pt_suc;
    bool eh_inedito;
    double compr;
    IDPonto id_pt_ant;
    list<Noh>::iterator old, big;
    //laco principal do algoritmo
    do{
        //leh e exclui o 1 Noh (o de menor custo) de aberto
        atual = Aberto.front();
        Aberto.pop_front();
        //inclui "atual" em Fechado (no inicio ou outra posicao)
        Fechado.push_back(atual);
        //expande se não é solucao
        if(atual.id_pt!=id_destino){
            //gera sucessores de "atual"
            itr_rotas = rotas.begin();
            while(itr_rotas!=rotas.end()){
                //busca "rota_suc", proxima rota conectada a "atual"
                itr_rotas = find_if(itr_rotas,rotas.end(),[&atual](const Rota& R){return (R.extremidade[0]==atual.id_pt || R.extremidade[1]==atual.id_pt);}); //CLASSE IDPONTO PRECISA DO OPERATOR==(IDPONTO)
                //Achou uma rota?
                if(itr_rotas!=rotas.end()){
                    rota_suc = *itr_rotas;
                    //Gera Noh sucessor "suc"
                    try
                    {
                        suc.id_pt = rota_suc.outraExtremidade(atual.id_pt);
                        //Ponto do Noh "suc"
                        pt_suc = getPonto(suc.id_pt);
                    }
                    catch(...)
                    {
                        throw 4;
                    }
                    suc.id_rt = rota_suc.id;
                    suc.g = atual.g + rota_suc.comprimento;
                    try
                    {
                        suc.h = pt_suc.distancia(pt_dest);
                    }
                    catch(...)
                    {
                        throw 5;
                    }
                    //inicialmente, assume que nao existe igual a suc
                    eh_inedito = true;
                    //procura Noh igual a "suc" em fechado
                    old = find(Fechado.begin(),Fechado.end(),suc.id_pt); //CLASSE NOH PRECISA DO OPERATOR==(IDPONTO)
                    //Achou algum Noh?
                    if(old!=Fechado.end()){
                        //Noh já existe
                        eh_inedito = false;
                    }else{
                        //procura Noh igual a "suc" em Aberto
                        old = find(Aberto.begin(),Aberto.end(),suc.id_pt); //CLASSE NOH PRECISA DO OPERATOR==(IDPONTO)
                        //Achou algum Noh?
                        if(old!=Aberto.end()){
                            //Menor custo total?
                            if(suc.f() < old->f()){
                                //Exclui anterior
                                Aberto.erase(old);
                            }else{
                                //Noh já existe
                                eh_inedito = false;
                            }
                        }
                    }
                    //Nao existe igual?
                    if(eh_inedito){
                        //Acha "big", 1 Noh de Aberto com custo total f() maior que o custo total f() de "suc"
                        big = upper_bound(Aberto.begin(),Aberto.end(),suc); //CLASSE NOH PRECISA DO OPERADOR<(NOH)
                        //Insere "suc" em Aberto antes de "big"
                        Aberto.insert(big,suc);
                    }
                    ++itr_rotas;
                }
            }
        }
    }while(!Aberto.empty() && atual.id_pt!=id_destino);
    //Calcula num de nohs da busca
    NA = static_cast<int>(Aberto.size());
    NF = static_cast<int>(Fechado.size());
    //Inicialmente, caminho vazio
    //(ele já está vazio)
    //encontrou solucao ou nao?
    if(atual.id_pt!=id_destino){
        //Nao existe solucao
        compr = -1;
        //caminho permanece vazio
    }else{
        //calcula comprimento do caminho
        compr = atual.g;
        //Refaz o caminho, procurando Nohs antecessores em Fechado
        while(atual.id_rt.valid()){
            //Acrescenta Trecho atual no topo (inicio) de "caminho"
            C.push_front(Trecho(atual.id_rt,atual.id_pt));
            //Recupera o antecessor.
            //Obtem "rota_ant", Rota que levou até atual.
            try
            {
                rota_ant = getRota(atual.id_rt);
                //Calcula id do antecessor
                id_pt_ant = rota_ant.outraExtremidade(atual.id_pt);
            }
            catch(...)
            {
                throw 6;
            }
            //Procura Noh igual a "id_pt_ant" em Fechado)
            atual = *(find(Fechado.begin(),Fechado.end(),id_pt_ant)); //CLASSE NOH PRECISA DO OPERATOR==(IDPONTO)
        }
        //Acrescenta origem no topo (inicio) de "caminho"
        C.push_front(Trecho(IDRota(),atual.id_pt));
    }

    // O try tem que terminar retornando o comprimento calculado
    return compr;  // SUBSTITUA pelo return do valor correto
  }
  catch(int i)
  {
    string msg_err = "Erro " + to_string(i) + " no calculo do caminho\n";
    throw invalid_argument(msg_err);
  }
}
