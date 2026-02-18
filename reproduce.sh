#!/bin/bash

#Clean compilation files
./clean.sh

#Install required Python packages
pip install -r requirements.txt

#Download original Exiobase dataset
pip install -U zenodo-get
zenodo_get 10.5281/zenodo.15689391

#Build the benchmark suite
python build_benchmark_suite.py

#Create table for instance statistics
python analyze_instances.py

#Compile all algorithms
./compile_algorithms.sh

#Run experiments on rxr,pxp,os300
python run_experiment.py

#Create statistics from experiment results
python calculate_results_statistics.py

#Create graphs/tables for single solution analysis
python analyze_ss_results.py

#Create graphs/tables for multiple solution analysis
python analyze_ms_results.py

#Run experiments on isic
./run_isic_experiment1.sh
python run_isic_experiment2.py

#Create graphs/tables for isic (small) instances
python analyze_isic_results.py
