#!/bin/bash

for instance in exiobase/isic/*; do
	./exhaustive_evaluation $instance
done

