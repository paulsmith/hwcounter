#include <stddef.h>

#include "Python.h"
#include "structmember.h"

#define HWCOUNTER_GET_TIMESTAMP(count_ptr)				\
    do {								\
	uint32_t count_high, count_low;					\
	asm volatile (							\
		      "cpuid\n\t"					\
		      "rdtsc\n\t"					\
		      : "=a" (count_low), "=d" (count_high)		\
		      :: "ebx", "ecx");					\
	*count_ptr = ((uint64_t)count_high << 32) | count_low;		\
    } while(0)

#define HWCOUNTER_GET_TIMESTAMP_END(count_ptr)				\
    do {								\
	uint32_t count_high, count_low;					\
	asm volatile (							\
		      "rdtscp\n\t"					\
		      "mov %%edx, %0\n\t"				\
		      "mov %%eax, %1\n\t"				\
		      "cpuid\n\t"					\
		      : "=r" (count_high), "=r" (count_low)		\
		      :: "eax", "ebx", "ecx", "edx");			\
	*count_ptr = ((uint64_t)count_high << 32) | count_low;		\
    } while(0)

uint64_t
hwcounter_measure_overhead(void)
{
    uint64_t t0, t1, elapsed, overhead = ~0;

    for (int i = 0; i < 3; i++) {
	HWCOUNTER_GET_TIMESTAMP(&t0);
	asm volatile("");
	HWCOUNTER_GET_TIMESTAMP_END(&t1);
	elapsed = t1 - t0;
	if (elapsed < overhead)
	    overhead = elapsed;
    }

    return overhead;
}

static PyObject *
hwcounter_get_count(PyObject *self, PyObject *args)
{
    uint64_t ts;

    (void)self;
    (void)args;
    HWCOUNTER_GET_TIMESTAMP(&ts);
    return PyLong_FromUnsignedLongLong(ts);
}

static PyObject *
hwcounter_get_count_end(PyObject *self, PyObject *args)
{
    uint64_t ts;

    (void)self;
    (void)args;
    HWCOUNTER_GET_TIMESTAMP_END(&ts);
    return PyLong_FromUnsignedLongLong(ts);
}

typedef struct {
    PyObject_HEAD
    uint64_t t0;
    uint64_t overhead;
    uint64_t cycles;
} hwcounter_Timer;

static void
hwcounter_Timer_dealloc(hwcounter_Timer *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
hwcounter_Timer_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    hwcounter_Timer *t;

    t = (hwcounter_Timer *)type->tp_alloc(type, 0);
    if (t != NULL) {
	t->t0 = 0;
	t->overhead = 0;
	t->cycles = 0;
    }

    return (PyObject *)t;
}

static PyObject *
hwcounter_Timer___enter__(PyObject *self)
{
    hwcounter_Timer *timer = (hwcounter_Timer *)self;

    timer->overhead = hwcounter_measure_overhead();
    HWCOUNTER_GET_TIMESTAMP(&timer->t0);
    Py_INCREF(self);
    return self;
}

static PyObject *
hwcounter_Timer___exit__(PyObject *self, PyObject *args)
{
    uint64_t ts;
    hwcounter_Timer *timer = (hwcounter_Timer *)self;

    (void)args;
    HWCOUNTER_GET_TIMESTAMP_END(&ts);
    timer->cycles = ts - timer->t0 - timer->overhead;
    timer->t0 = timer->overhead = 0;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMemberDef hwcounter_Timer_members[] = {
    {"cycles", T_ULONGLONG, offsetof(hwcounter_Timer, cycles), 0, "cycles elapsed"},
    {NULL}
};

static PyMethodDef hwcounter_Timer_methods[] = {
    {"__enter__", (PyCFunction)hwcounter_Timer___enter__, METH_NOARGS, NULL},
    {"__exit__", (PyCFunction)hwcounter_Timer___exit__, METH_VARARGS, NULL},
    {NULL}
};

static PyTypeObject hwcounter_TimerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "hwcounter.Timer",         /* tp_name */
    sizeof(hwcounter_Timer),   /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)hwcounter_Timer_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "Timer for counting cycles on code execution blocks", /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    hwcounter_Timer_methods,   /* tp_methods */
    hwcounter_Timer_members,   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    hwcounter_Timer_new,       /* tp_new */
};

static PyMethodDef hwcounter_methods[] = {
    {"count", hwcounter_get_count, METH_NOARGS,
     "Get current count of cycles from the hardware timestamp counter."},
    {"count_end", hwcounter_get_count_end, METH_NOARGS,
     "Get current count of cycles from the hardware timestamp counter. Suitable for the end of a timing block."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef hwcounter_module = {
//    PyModuleDef_HEAD_INIT,
    PyObject_HEAD_INIT,
    "hwcounter",
    "This module provides access to a very accurate, high-resolution hardware counter for measurement of time, in terms of processor clock cycles.",
    -1,
    hwcounter_methods
};

PyMODINIT_FUNC
PyInit_hwcounter(void)
{
    PyObject *m;

    hwcounter_TimerType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&hwcounter_TimerType) < 0)
	return NULL;

    if ((m = PyModule_Create(&hwcounter_module)) == NULL)
	return NULL;

    Py_INCREF(&hwcounter_TimerType);
    PyModule_AddObject(m, "Timer", (PyObject *)&hwcounter_TimerType);

    return m;
}
