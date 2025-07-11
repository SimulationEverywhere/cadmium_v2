#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../modeling/devs/coupled.hpp"

namespace cadmium {
    // Recursively print the model tree
    inline void print_model_tree(const std::shared_ptr<Component>& component, const std::string& prefix = "", bool isLast = true) {
        // Print current component's ID
        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << component->getId() << "\n";

        // If it's a coupled model, print its subcomponents recursively
        auto coupled = std::dynamic_pointer_cast<Coupled>(component);
        if (coupled != nullptr) {
            const auto& children = coupled->getComponents();
            size_t count = 0;
            for (const auto& [childId, childPtr] : children) {
                bool childIsLast = (++count == children.size());
                print_model_tree(childPtr, prefix + (isLast ? "    " : "│   "), childIsLast);
            }
        }
    }
}