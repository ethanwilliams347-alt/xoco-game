#include "scene/scene.h"
#include "physics/grid.h"
#include "test_util.h"

int main() {
    {
        Grid g(10, 10, 123);
        Scene s;
        s.width = 2;
        s.height = 2;
        s.materials = {
            ElementType::Wall, ElementType::Sand,
            ElementType::Wood, ElementType::Water
        };
        s.albedo = {
            0xFF111111, 0xFF222222,
            0xFF333333, 0xFF444444
        };

        load_scene(g, s, 2, 2);

        check("scene Wall loaded correctly", g.get_element(2, 2).type == ElementType::Wall);
        check("scene Wall color loaded correctly", g.get_element(2, 2).color == 0xFF111111);
        
        check("scene Sand loaded correctly", g.get_element(3, 2).type == ElementType::Sand);
        check("scene Sand color loaded correctly", g.get_element(3, 2).color == 0xFF222222);

        check("scene Wood loaded correctly", g.get_element(2, 3).type == ElementType::Wood);
        check("scene Wood color loaded correctly", g.get_element(2, 3).color == 0xFF333333);

        check("scene Water loaded correctly", g.get_element(3, 3).type == ElementType::Water);
        check("scene Water color loaded correctly", g.get_element(3, 3).color == 0xFF444444);

        check("outside scene remains empty", g.get_element(1, 1).type == ElementType::Empty);
    }

    {
        // Empty is a background colour someone painted in an editor, not a
        // material with a legend entry - loading it must not stamp its albedo
        // pixel over whatever the grid already had there.
        Grid g(10, 10, 123);
        g.set_element(5, 5, ElementType::Wall);
        const uint32_t before = g.get_element(5, 5).color;

        Scene s;
        s.width = 1;
        s.height = 1;
        s.materials = { ElementType::Empty };
        s.albedo = { 0xFF999999 };

        load_scene(g, s, 5, 5);

        check("scene Empty leaves existing type untouched", g.get_element(5, 5).type == ElementType::Wall);
        check("scene Empty leaves existing color untouched", g.get_element(5, 5).color == before);
    }

    return report();
}
