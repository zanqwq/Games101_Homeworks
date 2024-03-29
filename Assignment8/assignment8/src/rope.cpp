#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        for (int i = 0; i < num_nodes; i++) {
            auto node_pos = start + (end - start) * ((double)i / (num_nodes - 1));
            masses.push_back(new Mass(node_pos, node_mass, false));

            if (i != 0) {
                springs.push_back(new Spring(masses[i - 1], masses[i], k));
            }
        }

//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            // 胡克定律: F_a2b = k * DirA2B * (||b - a|| - l)
            auto k = s->k;
            auto a = s->m1, b = s->m2;
            auto posA = a->position, posB = b->position;
            auto vecA2B = (posB - posA).unit();
            auto lenDelta = (posB - posA).norm() - s->rest_length;
            auto f_a = k * vecA2B * lenDelta;
            // why plus equal rather than equal, because a mass might be connected with two springs
            a->forces += f_a;
            b->forces += -f_a;

            // internal spring damping
            auto k_d = 0.01;
            auto f_d = -k_d * dot(b->velocity - a->velocity, vecA2B) * vecA2B;
            a->forces += f_d;
            b->forces += -f_d;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity * m->mass;

                // TODO (Part 2): Add global damping
                auto k_d = 0.01;
                m->forces += -k_d * m->velocity;

                auto acceleration = m->forces / m->mass;

                // 显示欧拉, 用当前速度算下一帧位置, 不稳定
                // m->position += m->velocity * delta_t;
                // m->velocity += acceleration * delta_t;

                // 隐式欧拉, 用下一帧速度算下一帧位置
                m->velocity += acceleration * delta_t;
                m->position += m->velocity * delta_t;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            auto b_a = s->m2->position - s->m1->position;
            auto f = s->k * b_a.unit() * (b_a.norm() - s->rest_length);
            s->m1->forces += f;
            s->m2->forces += -f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                m->forces += gravity * m->mass;
                auto acceleration = m->forces / m->mass;

                Vector2D next_last_pos = m->position;
                auto damping_factor = 0.0001;

                m->position = m->position + (1 - damping_factor) * (m->position - m->last_position) + acceleration * delta_t * delta_t;
                m->last_position = next_last_pos;
                // TODO (Part 3.1): Set the new position of the rope mass
                
                // TODO (Part 4): Add global Verlet damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }
}
