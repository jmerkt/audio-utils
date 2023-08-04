#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>

#include "../../include/SmoothedFloat.h"
#include "../../include/SmoothedOscillator.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace audio_utils;

PYBIND11_MODULE(audioutils, m)
{
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: audioutils

        .. autosummary::
           :toctree: _generate

    )pbdoc";

    py::class_<SmoothedFloat<double>>(m, "SmoothedFloat")
        .def(py::init<>())
        .def("init", &SmoothedFloat<double>::init)
        .def("setTargetValue", &SmoothedFloat<double>::setTargetValue)
        .def("setSmoothingTime", &SmoothedFloat<double>::setSmoothingTime)
        .def("getNextValue", &SmoothedFloat<double>::getNextValue)
        .def("getNextBlock", &SmoothedFloat<double>::Python_getNextBlock)
        .def("getCurrentValue", &SmoothedFloat<double>::getCurrentValue);

    py::class_<SmoothedOscillator>(m, "SmoothedOscillator")
        .def(py::init<const double &, const double &, const double &, const double &, const double &>())
        .def("init", &SmoothedOscillator::init)
        .def("setFrequency", &SmoothedOscillator::setFrequency)
        .def("setGain", &SmoothedOscillator::setGain)
        .def("getGain", &SmoothedOscillator::getGain)
        .def("processSample", &SmoothedOscillator::processSample)
        .def("processBlock", &SmoothedOscillator::Python_processBlock);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
