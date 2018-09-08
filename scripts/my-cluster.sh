#!/bin/bash
BIN_DIR=../bin
SRC_DIR=../src
pushd ${SRC_DIR} && make my-cluster; popd

PRETRAINED=/shared/preprocessed/qning2/summarization/word2vec/GoogleNews-vectors-negative300.bin
CLASSES_DATA=../output/classes-googlenews-pretrained
CLASSES_SORTED_DATA=../output/classes-googlenews-pretrained.sorted
KEEPVOCAB=../../data/vocab50000_reorder_trunc.txt
for NCLASS in 1500
do
	echo $NCLASS
    if [ ! -e CLASSES_DATA ]; then
    	echo "Generating class file"
        time $BIN_DIR/my-cluster $PRETRAINED ${CLASSES_DATA}.K$NCLASS $KEEPVOCAB $NCLASS 100
        sort ${CLASSES_DATA}.K$NCLASS -k 2 -n > $CLASSES_SORTED_DATA.K$NCLASS
    fi
    if [ ! -e CLASSES_SORTED_DATA ]; then
    	echo "Sorting class file"
        sort ${CLASSES_DATA}.K$NCLASS -k 2 -n > $CLASSES_SORTED_DATA.K$NCLASS
    fi
done