#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from statistics import mean
from joblib import dump, load

from sklearn.datasets import load_svmlight_file

from sklearn.preprocessing import StandardScaler
from mlxtend.feature_selection import ColumnSelector
from sklearn.preprocessing import PolynomialFeatures
from sklearn.preprocessing import QuantileTransformer

from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import GradientBoostingClassifier
from sklearn.svm import SVC
from sklearn.naive_bayes import GaussianNB
from sklearn.neighbors import KNeighborsClassifier
from xgboost import XGBClassifier
from mlxtend.classifier import EnsembleVoteClassifier

from sklearn.pipeline import Pipeline

from sklearn.metrics import f1_score, precision_score, recall_score, accuracy_score, SCORERS
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import ShuffleSplit

from sklearn.model_selection import learning_curve

def plot_learning_curve(estimator, title, X, y, ylim=None, cv=None,
                        n_jobs=None, train_sizes=np.linspace(.1, 1.0, 5)):
    plt.figure()
    plt.title(title)
    if ylim is not None:
        plt.ylim(*ylim)
    plt.xlabel("Training examples")
    plt.ylabel("Score")
    train_sizes, train_scores, test_scores = learning_curve(
        estimator, X, y, cv=cv, n_jobs=n_jobs, train_sizes=train_sizes)
    train_scores_mean = np.mean(train_scores, axis=1)
    train_scores_std = np.std(train_scores, axis=1)
    test_scores_mean = np.mean(test_scores, axis=1)
    test_scores_std = np.std(test_scores, axis=1)
    plt.grid()

    plt.fill_between(train_sizes, train_scores_mean - train_scores_std,
                     train_scores_mean + train_scores_std, alpha=0.1,
                     color="r")
    plt.fill_between(train_sizes, test_scores_mean - test_scores_std,
                     test_scores_mean + test_scores_std, alpha=0.1, color="g")
    plt.plot(train_sizes, train_scores_mean, 'o-', color="r",
             label="Training score")
    plt.plot(train_sizes, test_scores_mean, 'o-', color="g",
             label="Cross-validation score")

    plt.legend(loc="best")
    return plt



data = load_svmlight_file('dataset.svmlight')
x = data[0].todense()
y = data[1]

GNB = Pipeline([
    ('columnselector', ColumnSelector(cols=(0, 1, 2, 3, 5, 6))),
    #('polyfeat',PolynomialFeatures(degree=2)),
    ('qtrans', QuantileTransformer(n_quantiles=5, output_distribution='uniform')),
    ('classifier', GaussianNB())
    ])

SVM = Pipeline([
    ('columnselector', ColumnSelector(cols=(1, 2, 3, 5, 6))),
    ('polyfeat',PolynomialFeatures(degree=3)),
    ('qtrans', QuantileTransformer(n_quantiles=3, output_distribution='uniform')),
    ('classifier', SVC(kernel='poly', C=0.6, gamma=0.188, degree=3, shrinking=True))
    ])

KNN = Pipeline([
    ('columnselector', ColumnSelector(cols=(0, 1, 2, 3, 5, 6))),
    #('polyfeat',PolynomialFeatures(degree=2)),
    ('qtrans', QuantileTransformer(n_quantiles=5, output_distribution='uniform')),
    ('classifier', KNeighborsClassifier(weights='uniform', p=3, n_neighbors=13))
    ])

XGB = Pipeline([
    ('columnselector', ColumnSelector(cols=(0, 1, 2, 3, 5, 6))),
    #('polyfeat',PolynomialFeatures(degree=2)),
    ('qtrans', QuantileTransformer(n_quantiles=10, output_distribution='normal')),
    ('classifier', XGBClassifier(min_child_weight=9, subsample=0.60, n_estimators=92, max_depth=1, ))
    ])


model = Pipeline([
#('column_selector', ColumnSelector(cols=(0,1,2,3,4,5,6))),
('scaler', StandardScaler()),
#('poly', PolynomialFeatures(degree=3)),
#('quantile', QuantileTransformer(n_quantiles=5, output_distribution='uniform')),
#('classifier', LogisticRegression(penalty='l2', solver='liblinear'))
#('classifier', GradientBoostingClassifier())
#('classifier', SVC(kernel='poly', C=0.6, gamma=0.188, degree=3, shrinking=True))
#('classifier', GaussianNB())
#('classifier', KNeighborsClassifier(n_neighbors=13))
#('classifier', XGBClassifier(max_depth=3))
('classifier', EnsembleVoteClassifier(
    clfs=[
        GNB,
        KNN,
        XGB,
        SVM],
    weights=[0.5, 0.5, 0.5, 1]))
])

scores  = mean(cross_val_score(
    estimator=model,
    X=x, y=y,
    cv=ShuffleSplit(n_splits=100, test_size=0.3), scoring='f1'))

print("score: %0.2f (+/- %0.5f)" % (scores.mean(), scores.std() * 2))

model.fit(x, y)
dump(model, 'classifier.joblib')
y_pred = model.predict(x)
print('accuracy: ', accuracy_score(y, y_pred))
print('f1: ', f1_score(y, y_pred))
print('precision: ', precision_score(y, y_pred))
print('recall: ', recall_score(y, y_pred))


#plot_learning_curve(
#    model, 'Learning Curve', x, y,
#    train_sizes=[np.linspace(0.1, 1, 100)],
#    cv=5).show()



