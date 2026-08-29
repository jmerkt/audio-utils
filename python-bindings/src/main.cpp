#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../../include/smoothed_float.h"
#include "../../include/smoothed_oscillator.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
using namespace audio_utils;

PYBIND11_MODULE(audioutils, module)
{
    module.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: audioutils

        .. autosummary::
           :toctree: _generate

    )pbdoc";

    py::class_<SmoothedFloat<double>>(module, "SmoothedFloat")
        .def(py::init<>())
        .def("init", &SmoothedFloat<double>::init)
        .def("set_target_value", &SmoothedFloat<double>::set_target_value)
        .def("set_smoothing_time", &SmoothedFloat<double>::set_smoothing_time)
        .def("get_next_value", &SmoothedFloat<double>::get_next_value)
        .def("get_next_block", &SmoothedFloat<double>::python_get_next_block)
        .def("get_current_value", &SmoothedFloat<double>::get_current_value);

    py::class_<SmoothedOscillator>(module, "SmoothedOscillator")
        .def(py::init<const double &, const double &, const double &, const double &, const double &>())
        .def("init", &SmoothedOscillator::init)
        .def("set_frequency", &SmoothedOscillator::set_frequency)
        .def("set_gain", &SmoothedOscillator::set_gain)
        .def("get_gain", &SmoothedOscillator::get_gain)
        .def("process_sample", &SmoothedOscillator::process_sample)
        .def("process_block", &SmoothedOscillator::python_process_block);

#ifdef VERSION_INFO
    module.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    module.attr("__version__") = "dev";
#endif
}
