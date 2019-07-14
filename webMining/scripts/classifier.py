#!/usr/bin/python3
from joblib import dump, load
import numpy

model = load('classifier.joblib')

def predict(x):
	return model.predict(numpy.matrix([x]))



