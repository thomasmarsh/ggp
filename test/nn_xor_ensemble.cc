#include <engine/neural.h>
#include <engine/common.h>
#include <ui/nn.h>

static const double LEARNING_RATE = 0.2;

int main() {
        srandomdev();
        srand(time(NULL));
        typedef EnsembleFFNet<4,2,2,1,Tanh,Tanh> NN;
        typedef NN::Query Q;

        Q q;
        q.input << 0.0 << 0.0 << 1.0 << arma::endr
                << 0.0 << 1.0 << 1.0 << arma::endr
                << 1.0 << 0.0 << 1.0 << arma::endr
                << 1.0 << 1.0 << 1.0 << arma::endr;

        q.target << -0.0 << arma::endr
                 <<  1.0 << arma::endr
                 <<  1.0 << arma::endr
                 << -0.0 << arma::endr;

        NN net;
        net.momentum = 0.0;

        q.input.print("q.input");
        q.target.print("q.target");
        double mse = 1;
        HintonDiagram hinton(300, 300);
        while (mse > .0000001) {
                mse = net.train_one(q, LEARNING_RATE);
                LOG("mse=" << mse);

                Q q2 = q;
                net.fprop(q2);
                q2.output.print("output:");
                hinton.update(net.net[0].w1, net.net[0].w2);
        }
}
