struct CNN {
        arma::mat weights;

        CNN(size_t in, size_t out, size_t filt) {
                weights = arma::randu(in, out) * .005;
        }
};

struct DNN {
        FFN<10,Softmax>
        FFN<150,Tanh> 
        MaxPool<3,3>
        CNN<40,5>
        MaxPool<2,2>
        CNN<20,4>


        // STRUCTURE:
        // ----------
        //
        // 1x29x29 - input layer
        // 20C4    - 20 map convolution with 4x4 filters
        // MP2     - max pool over non-overlapping regions of 2x2
        // 40C5    - 40 map convolution with 5x5 filters
        // MP3     - max pool over non-overlapping regions of 3x3
        // 150N    - fully connected 150 neurons
        // 10N     - fully connected output layer 10 neurons
        //
        //
        // ACTIVATION:
        // -----------
        //
        // scaled hyperbolic tangent for convolution and fully connected
        // linear activation for max-pooling
        // softmax for output
        //
        //
        // TRAINING:
        // ---------
        //
        // annealed learning rate (0.001 * 0.993/epoch until 0.00003)
        // [takes 14 hours and little improvement after 500 epochs]
};
