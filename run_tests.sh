#!/bin/bash
set -e

# parametros (numeros inteiros)
# -----------------------------
# evitar valores muito grandes de tempo (overflow nao verificado)
# tempo total de execucao do experimento: (2 * (SAMPLES * SEC)) segundos
SEC="10"
SAMPLES="20"


# definicao de funcoes
# --------------------
die() {
  # imprime seus argumentos na saida padrao de erros (2) e executa 'exit 1'
  echo "${@}" >&2;
  exit 1;
}

check() {
  # verifica se seu argumento e' um executavel reconhecido pela shell
  type -a ${@} > /dev/null 2>&1 || die "${@} nao encontrado no path."
}

run() {
  # executa a implementacao recebida como argumento SAMPLES vezes durante SEC segundos
  for ((i=0; i < ${SAMPLES}; i++)) {
    # imprime o progresso no formato "implementacao (iter_concluidas)/(total_iter)"
    echo -ne "\r${1} ${i}/${SAMPLES}"
    # executa produtor e consumidor em "background"
    ./bin/prod-${1} > prod.tmp &
    PROD_PID=$!
    ./bin/cons-${1} > cons.tmp &
    CONS_PID=$!
    # envia SIGTERM aos programas apos SEC segundos
    sleep ${SEC} && kill ${PROD_PID} ${CONS_PID}
    # anexa resultados ao arquivo de saida
    echo -e "$(cat prod.tmp)\n$(cat cons.tmp)" >> ./data/${1}.csv
  }
  echo -e "\r${1} ${SAMPLES}/${SAMPLES}"
}

plot() {
  # gera um grafico comparativo com as saidas das duas implementacoes
  COLOR_LCK="#aa0e50"
  COLOR_SEM="#516288"

  gnuplot << EndOfFile
  set datafile separator ","
  set title "spin lock x semaphore"
  set terminal svg enhanced size 500,500 background rgb 'white'

  set output "plot_comparacao.svg"
  set bmargin 3
  set xlabel "Dados (kB)"
  set ylabel "Tempo (s)"
  #set xtics 1
  #set ytics 0.02
  #set yrange [0:0.5]
  #set pointintervalbox 3
  set grid

  set style line 1 lc rgb '$COLOR_LCK' lt 1 lw 1 pt 7 ps 0.5
  set style line 2 lc rgb '$COLOR_SEM' lt 1 lw 1 pt 7 ps 0.5
  plot "./data/lck.csv" with line ls 1 t 'spin lock',\
       "./data/sem.csv" with line ls 2 t 'semaphore'
EndOfFile
}


# "entry point" do script (a execucao comeca aqui)
# ------------------------------------------------
# executa cada implementacao se o script nao foi executado com o argumento 'plot'
if [[ $1 != "plot" ]]; then
  check make

  # tenta executar o Makefile do projeto
  make -s || die "[ERROR]: make retornou codigo ${?}."

  # cria o diretorio data caso nao exista
  [ -d ./data ] || mkdir ./data

  # inicializa arquivos de saida
  echo "# bytes,cpu_time@${SEC}s" > ./data/sem.csv
  echo "# bytes,cpu_time@${SEC}s" > ./data/lck.csv
  run lck
  run sem
fi

# remove arquivos temporarios caso existam
[ -f prod.tmp ] && rm prod.tmp
[ -f cons.tmp ] && rm cons.tmp

check gnuplot
plot
