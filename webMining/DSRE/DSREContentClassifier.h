/*
 * DSREContentClassifier.h
 *
 *  Created on: 13 de jul de 2019
 *      Author: roberto
 */

#ifndef DSRE_DSRECONTENTCLASSIFIER_H_
#define DSRE_DSRECONTENTCLASSIFIER_H_

#include <Python.h>

class DSREContentClassifier {
public:
	DSREContentClassifier();
	virtual ~DSREContentClassifier();
	bool predict(
		float positionScore,
		float sizeScore,
		float recScore,
		float rangeScore,
		float angleScore,
		float endHScore,
		float endVScore);
private:
	PyObject *pFunc;

	void loadPythonScript();
};

#endif /* DSRE_DSRECONTENTCLASSIFIER_H_ */
