#include <ui/ui.h>

struct Render {
        void render(Window &w) {
                w.color(1, .5, 0);
                w.square(10, 10, 20, 20);
                usleep(50);
        }
};

int main() {
        Window w;
        w.open("test", 300, 300);
        Render r;
        w.loop(r);
        w.close();
}
