#!/bin/bash
set -e

# parametros (numeros inteiros)
# -----------------------------
# evitar valores muito grandes de tempo (overflow nao verificado)
# tempo total de execucao do experimento: (2 * (SAMPLES * SEC)) segundos
SEC="15"
SAMPLES="15"


# definicao de funcoes
# --------------------
die() {
  # imprime seus argumentos na saida padrao de erros (2) e executa 'exit 1'
  echo "[ERROR]: ${@}" >&2;
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
    ./bin/prod-${1} > /tmp/prod.tmp &
    PROD_PID=$!
    ./bin/cons-${1} > /tmp/cons.tmp &
    CONS_PID=$!
    # envia SIGTERM aos programas apos SEC segundos
    sleep ${SEC} && kill ${PROD_PID} ${CONS_PID}
    # anexa resultados ao arquivo de saida
    echo -e "$(cat /tmp/prod.tmp)\n$(cat /tmp/cons.tmp)" >> ./data/${1}.csv
  }
  echo -e "\r${1} ${SAMPLES}/${SAMPLES}"
}

plot() {
  # gera um grafico comparativo com as saidas das duas implementacoes
  COLOR_LCK="#aa0e50"
  COLOR_SEM="#516288"

  gnuplot << EndOfFile
  set datafile separator ","
  set title "Spin Lock x Semaphore at ${SEC}s real time"
  set terminal svg enhanced size 500,500 background rgb 'white'

  set output "/dev/null"
  plot "./data/lck.csv" with points ls 1 t 'spin lock',\
       "./data/sem.csv" with points ls 2 t 'semaphore'

  set output "plot.svg"
  set yrange [GPVAL_Y_MIN:GPVAL_DATA_Y_MAX]

  set bmargin 3
  set xlabel "Data (MiB)"
  set ylabel "CPU time (s)"
  set grid
  set key right bottom

  set style line 1 lc rgb '$COLOR_LCK' lt 1 lw 1 pt 7 ps 0.5
  set style line 2 lc rgb '$COLOR_SEM' lt 1 lw 1 pt 7 ps 0.5
  plot "./data/lck.csv" with points ls 1 t 'spin lock',\
       "./data/sem.csv" with points ls 2 t 'semaphore'
EndOfFile
}


# "entry point" do script (a execucao comeca aqui)
# ------------------------------------------------
# executa cada implementacao se o script nao foi executado com o argumento 'plot'
if [[ ${1} != "plot" ]]; then
  check make

  # tenta executar o Makefile do projeto
  make || die "make retornou codigo ${?}."

  # cria o diretorio data caso nao exista
  [ -d ./data ] || mkdir ./data

  # inicializa arquivos de saida
  echo "# MiB,cpu_time@${SEC}s" > ./data/sem.csv
  echo "# MiB,cpu_time@${SEC}s" > ./data/lck.csv
  run lck
  run sem
fi

check gnuplot
plot
