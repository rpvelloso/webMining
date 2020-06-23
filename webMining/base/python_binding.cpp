#include "../DSRE/DSRE.hpp"
#include "DOM.hpp"
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;


BOOST_PYTHON_MODULE(webMining) {
    class_<std::vector<int> >("IntVec")
        .def(vector_indexing_suite<std::vector<int> >());

    class_<std::vector<double> >("DoubleVec")
        .def(vector_indexing_suite<std::vector<double> >());

    class_<DOM, boost::noncopyable>("DOM", init<const std::string &, const std::string &>())
        .def("printHTML", &DOM::printHTML)
        .def("getURI", &DOM::getURI);

    class_<DSRE, boost::noncopyable>("DSRE", init<>())
        .def("extract", &DSRE::extract)
        .def("setMinPSD", &DSRE::setMinPSD)
        .def("setMinCV", &DSRE::setMinCV)
        .def("setUseFourier", &DSRE::setUseFourier)
        .def("getTps", &DSRE::getTps)
        .def("regionCount", &DSRE::regionCount)
        .def("getSequence", &DSRE::getSequence)
        .def("getDataRegion", &DSRE::getDataRegion);

     class_<DSREDataRegion>("DSREDataRegion", no_init)
        .def("recordCount", &DSREDataRegion::recordCount)
        .def("recordSize", &DSREDataRegion::recordSize)
        .def("isContent", &DSREDataRegion::isContent)
        .def("getSequence", &DSREDataRegion::getSequence)
        .def("getTransform", &DSREDataRegion::getTransform)
        .def("getCell", &DSREDataRegion::getCell, return_value_policy<reference_existing_object>());

    class_<Node, boost::noncopyable>("Node", no_init)
        .def("toString", &Node::toString);

    
}
