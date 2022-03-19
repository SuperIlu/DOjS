/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <errno.h>
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DOjS.h"

#include "doublearray.h"
#include "neural.h"
#include "genann.h"

/************
** structs **
************/
//! userdata definition
typedef struct __sqlite {
    genann *ann;      //!< the network
    double *inputs;   //!< buffer for input values
    double *outputs;  //!< buffer for output values
} neural_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize a database and free resources.
 *
 * @param J VM state.
 */
static void Neural_Finalize(js_State *J, void *data) {
    neural_t *n = (neural_t *)data;
    if (n->ann) {
        DEBUGF("FREE n->ann = %p\n", n->ann);
        genann_free(n->ann);
        n->ann = NULL;
    }
    DEBUGF("FREE n->in = %p\n", n->inputs);
    DEBUGF("FREE n->out = %p\n", n->outputs);
    free(n->inputs);
    free(n->outputs);
    DEBUGF("FREE n = %p\n", n);
    free(n);
}

/**
 * @brief create a network
 * ann = new Neural(inputs, # hidden layers, hidden, outputs)
 * ann = new Neural(filename:string)
 *
 * @param J VM state.
 */
static void new_Neural(js_State *J) {
    NEW_OBJECT_PREP(J);

    /* This will make the neural network initialize differently each run. */
    /* If you don't get a good result, try again for a different result. */
    srand(time(0));

    // get mem for user struct
    neural_t *n = malloc(sizeof(neural_t));
    if (!n) {
        JS_ENOMEM(J);
        return;
    }
    DEBUGF("n = %p\n", n);

    // open or create AN network
    if (js_isnumber(J, 1) && js_isnumber(J, 2) && js_isnumber(J, 3) && js_isnumber(J, 4)) {
        n->ann = genann_init(js_touint32(J, 1),  // inputs
                             js_touint32(J, 2),  // hidden layers
                             js_touint32(J, 3),  // hidden
                             js_touint32(J, 4)   // outputs
        );
        if (!n->ann) {
            free(n);
            JS_ENOMEM(J);
            return;
        }
    } else {
        const char *fname = js_tostring(J, 1);
        FILE *f = fopen(fname, "r");
        if (!f) {
            js_error(J, "cannot open file '%s': %s", fname, strerror(errno));
            free(n);
            return;
        }

        n->ann = genann_read(f);

        fclose(f);

        if (!n->ann) {
            js_error(J, "Could not read neural network from '%s'", fname);
            free(n);
            return;
        }
    }

#ifdef DEBUG_ENABLED
    DEBUGF("n->ann = %p\n", n->ann);
    const int size = sizeof(genann) + sizeof(double) * (n->ann->total_weights + n->ann->total_neurons + (n->ann->total_neurons - n->ann->inputs));
    DEBUGF("sizeof(n->ann) = %d\n", size);
#endif

    // alloc buffers for train() and run()
    n->inputs = calloc(n->ann->inputs, sizeof(double));
    n->outputs = calloc(n->ann->outputs, sizeof(double));
    if (!n->inputs || !n->outputs) {
        genann_free(n->ann);
        free(n->inputs);
        free(n->outputs);
        free(n);
        JS_ENOMEM(J);
        return;
    }
    DEBUGF("n->in = %p\n", n->inputs);
    DEBUGF("n->out = %p\n", n->outputs);

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_NEURAL, n, Neural_Finalize);

    // add properties
    js_pushnumber(J, n->ann->inputs);
    js_defproperty(J, -2, "inputs", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, n->ann->outputs);
    js_defproperty(J, -2, "outputs", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, n->ann->hidden);
    js_defproperty(J, -2, "hidden", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, n->ann->hidden_layers);
    js_defproperty(J, -2, "hidden_layers", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close the network.
 * ann.Close()
 *
 * @param J VM state.
 */
static void Neural_Close(js_State *J) {
    neural_t *n = js_touserdata(J, 0, TAG_NEURAL);
    if (n->ann) {
        genann_free(n->ann);
        n->ann = NULL;
    }
}

/**
 * @brief run a number of inputs through the ANN and generate output.
 * ann.Run(in:number[]):number[]
 *
 * @param J VM state.
 */
static void Neural_Run(js_State *J) {
    neural_t *n = js_touserdata(J, 0, TAG_NEURAL);
    if (n->ann) {
        // input array
        if (js_isuserdata(J, 1, TAG_DOUBLE_ARRAY)) {
            double_array_t *ia = js_touserdata(J, 1, TAG_DOUBLE_ARRAY);
            if (ia->size < n->ann->inputs) {
                js_error(J, "Parameter 1 needs to be a DoubleArray of length >= %d", n->ann->inputs);
                return;
            }

            for (int i = 0; i < n->ann->inputs; i++) {
                n->inputs[i] = ia->data[i];
            }
        } else {
            if (!js_isarray(J, 1) || (js_getlength(J, 1) < n->ann->inputs)) {
                js_error(J, "Parameter 1 needs to be a Javascript array of length >= %d", n->ann->inputs);
                return;
            }

            for (int i = 0; i < n->ann->inputs; i++) {
                js_getindex(J, 1, i);
                n->inputs[i] = js_tonumber(J, -1);
                js_pop(J, 1);
            }
        }

        // run it
        const double *res = genann_run(n->ann, n->inputs);

        js_newarray(J);
        for (int i = 0; i < n->ann->outputs; i++) {
            js_pushnumber(J, res[i]);
            js_setindex(J, -2, i);
        }
    } else {
        js_error(J, "Network was closed");
    }
}

/**
 * @brief train network with given data
 * ann.Train(in:number[], expected_out:number[], rate:number)
 *
 * @param J VM state.
 */
static void Neural_Train(js_State *J) {
    neural_t *n = js_touserdata(J, 0, TAG_NEURAL);
    if (n->ann) {
        // input array
        if (js_isuserdata(J, 1, TAG_DOUBLE_ARRAY)) {
            double_array_t *ia = js_touserdata(J, 1, TAG_DOUBLE_ARRAY);
            if (ia->size < n->ann->inputs) {
                js_error(J, "Parameter 1 needs to be a DoubleArray of length >= %d", n->ann->inputs);
                return;
            }

            for (int i = 0; i < n->ann->inputs; i++) {
                n->inputs[i] = ia->data[i];
            }
        } else {
            if (!js_isarray(J, 1) || (js_getlength(J, 1) < n->ann->inputs)) {
                js_error(J, "Parameter 1 needs to be a Javascript array of length >= %d", n->ann->inputs);
                return;
            }

            for (int i = 0; i < n->ann->inputs; i++) {
                js_getindex(J, 1, i);
                n->inputs[i] = js_tonumber(J, -1);
                js_pop(J, 1);
            }
        }

        // output array
        if (js_isuserdata(J, 2, TAG_DOUBLE_ARRAY)) {
            double_array_t *ia = js_touserdata(J, 2, TAG_DOUBLE_ARRAY);
            if (ia->size < n->ann->outputs) {
                js_error(J, "Parameter 2 needs to be an array of length >= %d", n->ann->outputs);
                return;
            }

            for (int i = 0; i < n->ann->outputs; i++) {
                n->outputs[i] = ia->data[i];
            }
        } else {
            if (!js_isarray(J, 2) || (js_getlength(J, 2) < n->ann->outputs)) {
                js_error(J, "Parameter 2 needs to be an array of length >= %d", n->ann->outputs);
                return;
            }
            for (int i = 0; i < n->ann->outputs; i++) {
                js_getindex(J, 2, i);
                n->outputs[i] = js_tonumber(J, -1);
                js_pop(J, 1);
            }
        }

        // learning rate
        double rate = js_tonumber(J, 3);
        if (rate <= 0) {
            js_error(J, "Learning rate must be >0 (was %f)!", rate);
            return;
        }

        // train it
        genann_train(n->ann, n->inputs, n->outputs, rate);
    } else {
        js_error(J, "Network was closed");
    }
}

/**
 * @brief store training data into file.
 * ann.Save(file_name:string)
 *
 * @param J VM state.
 */
static void Neural_Save(js_State *J) {
    neural_t *n = js_touserdata(J, 0, TAG_NEURAL);
    if (n->ann) {
        const char *fname = js_tostring(J, 1);
        FILE *f = fopen(fname, "w");
        if (!f) {
            js_error(J, "cannot open file '%s': %s", fname, strerror(errno));
            free(n);
            return;
        }

        genann_write(n->ann, f);

        fclose(f);
    } else {
        js_error(J, "Network was closed");
    }
}

/**
 * @brief get all data in the network from last run.
 *
 * @param J VM state.
 */
static void Neural_GetAllData(js_State *J) {
    neural_t *n = js_touserdata(J, 0, TAG_NEURAL);
    if (n->ann) {
        js_newarray(J);
        for (int i = 0; i < n->ann->total_neurons; i++) {
            js_pushnumber(J, n->ann->output[i]);
            js_setindex(J, -2, i);
        }
    } else {
        js_error(J, "Network was closed");
    }
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_neural(js_State *J) {
    init_doublearray(J);

    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, Neural, Close, 0);
        NPROTDEF(J, Neural, Run, 1);
        NPROTDEF(J, Neural, Train, 1);
        NPROTDEF(J, Neural, Save, 1);
        NPROTDEF(J, Neural, GetAllData, 0);
    }
    CTORDEF(J, new_Neural, TAG_NEURAL, 4);
}
