#include "neural.h"

//typedef FFNet<3,6,3,Sigmoid,Sigmoid> NET;
typedef FFNet<1,3,1,Sigmoid,Linear> NET;
//typedef FFNet<1,1,1,Sigmoid,Linear> NET;


void target1(NET::Query &q, vector<float> &target) {
	target[0] = sin(q.input[0]);//+ q.input[1]);
	//target[1] = cos(q.input[1]);
//	target[2] = q.input[0] *target[1];
}

static const int EPOCH_LEN = 200000;

int main() {
        srandomdev();
        NET net;
        NET::Query q;
	srand(time(NULL));
	unsigned iter = 0;
        vector<float> target(1,0);

	while (true) {
		float mse = 0;
		for (int i=0; i < EPOCH_LEN; i++) {
			q.input[0] = randf()*2-1;
			q.input[1] = randf()*2-1;

			net.propagate(q);

			target1(q, target);
                	mse += net.train(q, target, 1);
		}
		mse /= float(EPOCH_LEN);
                if (isnan(mse))
                        net.randomize();
		//net.print(q);
		printf(" MSE=%f\n", mse);
		iter++;
		if (mse < 0.0000005) break;
        }
	printf("stopped at %d iterations\n", iter);
}
