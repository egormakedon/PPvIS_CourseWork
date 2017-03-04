extern "C"
{
    #include "sc_memory_headers.h"
    #include "sc_helper.h"
    #include "utils.h"
}

#include <stdio.h>
#include <iostream>
#include <glib.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;

sc_memory_context *context;

sc_addr graph, rrel_arcs, rrel_nodes, rrel_parent, visit;

int MaxCycle=-1;
int kol=0;

sc_bool find_vertex_in_set(sc_addr element, sc_addr set)
{
    sc_bool find = SC_FALSE;

    sc_iterator3 *location = sc_iterator3_f_a_a_new(context,
                             set,
                             sc_type_arc_pos_const_perm,
                             0);

    while (SC_TRUE == sc_iterator3_next(location))
    {
        sc_addr loc = sc_iterator3_value(location, 2);

        if (SC_ADDR_IS_NOT_EQUAL(loc, element))
        {
            find = SC_FALSE;
            continue;
        }

        else
        {
            find = SC_TRUE;
            break;
        }
    }

    return find;
}

void get_edge_vertexes(sc_addr edge, sc_addr &v1, sc_addr &v2)
{
    sc_memory_get_arc_begin(context, edge, &v1);
    sc_memory_get_arc_end(context, edge, &v2);
}

void print_graph()
{
    sc_addr arcs, nodes, loc, v1, v2, printed_vertex;
    sc_bool find;

    printed_vertex = sc_memory_node_new(context, sc_type_const);

    printEl(context, graph);
    cout << endl << "----------------------" << endl;

    sc_iterator5 *it = sc_iterator5_f_a_a_a_f_new(context,
                       graph,
                       sc_type_arc_pos_const_perm,
                       0,
                       sc_type_arc_pos_const_perm,
                       rrel_arcs);

    if (SC_TRUE == sc_iterator5_next(it))
    {
        arcs = sc_iterator5_value(it, 2);

        sc_iterator3 *arcs_it = sc_iterator3_f_a_a_new(context,
                                arcs,
                                sc_type_arc_pos_const_perm,
                                0);

        while (SC_TRUE == sc_iterator3_next(arcs_it))
        {
            sc_addr t_arc = sc_iterator3_value(arcs_it, 2);

            get_edge_vertexes(t_arc, v1, v2);

            printEl(context, v1);
            printf(" --> ");
            printEl(context, v2);
            cout << endl;

            if (SC_FALSE == find_vertex_in_set(v1, printed_vertex))
                sc_memory_arc_new(context, sc_type_arc_pos_const_perm, printed_vertex, v1);

            if (SC_FALSE == find_vertex_in_set(v2, printed_vertex))
                sc_memory_arc_new(context, sc_type_arc_pos_const_perm, printed_vertex, v2);
        }

        sc_iterator3_free(arcs_it);
    }

    sc_iterator5_free(it);

    it = sc_iterator5_f_a_a_a_f_new(context,
                                    graph,
                                    sc_type_arc_pos_const_perm,
                                    0,
                                    sc_type_arc_pos_const_perm,
                                    rrel_nodes);

    if (SC_TRUE == sc_iterator5_next(it))
    {
        nodes = sc_iterator5_value(it, 2);

        sc_iterator3 *nodes_it = sc_iterator3_f_a_a_new(context,
                                 nodes,
                                 sc_type_arc_pos_const_perm,
                                 0);


        while (SC_TRUE == sc_iterator3_next(nodes_it))
        {
            sc_addr t_node = sc_iterator3_value(nodes_it, 2);

            find = find_vertex_in_set(t_node, printed_vertex);

            if (find == SC_FALSE)
            {
                printEl(context, t_node);
                cout << endl;
            }
        }

        sc_iterator3_free(nodes_it);
    }

    sc_iterator5_free(it);
}

void cycle_found(sc_addr cycle_st,sc_addr cycle_end)
{
    sc_addr arcs, v1, v2;

    sc_iterator3 *arcs_it = sc_iterator3_f_a_a_new(context,
                            rrel_parent,
                            sc_type_arc_pos_const_perm,
                            0);

    while (SC_TRUE == sc_iterator3_next(arcs_it))
    {
        sc_addr t_arc = sc_iterator3_value(arcs_it, 2);

        get_edge_vertexes(t_arc, v1, v2);

        if (SC_TRUE==SC_ADDR_IS_EQUAL(cycle_st, v1))
        {
            kol++;

            if (SC_TRUE==SC_ADDR_IS_EQUAL(cycle_end, v2))
            {
                if (MaxCycle<kol) MaxCycle=kol;
                break;
            }

            else if (SC_FALSE==SC_ADDR_IS_EQUAL(cycle_end, v2))
                cycle_found(v2, cycle_end);
        }
    }

    sc_iterator3_free(arcs_it);
}

void dfs(sc_addr t_node,sc_addr vertex)
{
    sc_addr arcs, v1, v2, arc_visit, arc1, arc2;

    arc_visit = sc_memory_arc_new(context, sc_type_arc_pos_const_perm, visit, t_node);

    arc1 = sc_memory_arc_new(context, sc_type_arc_pos_const_perm, t_node, vertex);
    arc2 = sc_memory_arc_new(context, sc_type_arc_pos_const_perm, rrel_parent, arc1);

    sc_iterator5 *it = sc_iterator5_f_a_a_a_f_new(context,
                       graph,
                       sc_type_arc_pos_const_perm,
                       0,
                       sc_type_arc_pos_const_perm,
                       rrel_arcs);

    if (SC_TRUE == sc_iterator5_next(it))
    {
        arcs = sc_iterator5_value(it, 2);

        sc_iterator3 *arcs_it = sc_iterator3_f_a_a_new(context,
                                arcs,
                                sc_type_arc_pos_const_perm,
                                0);

        while (SC_TRUE == sc_iterator3_next(arcs_it))
        {
            sc_addr t_arc = sc_iterator3_value(arcs_it, 2);

            get_edge_vertexes(t_arc, v1, v2);

            if (SC_TRUE==SC_ADDR_IS_EQUAL(v1, t_node))
            {
                if (SC_FALSE == find_vertex_in_set(v2, visit))
                    dfs(v2,t_node);

                else if ((SC_TRUE == find_vertex_in_set(v2, visit)))
                {
                    kol=0;
                    cycle_found(t_node, v2);
                }
            }
        }

        sc_iterator3_free(arcs_it);
    }

    sc_iterator5_free(it);

    sc_memory_element_free(context, arc_visit);

    sc_memory_element_free(context, arc2);
    sc_memory_element_free(context, arc1);
}

void CircumOfGraf()
{
    sc_addr nodes;

    sc_iterator5 *it = sc_iterator5_f_a_a_a_f_new(context,
                       graph,
                       sc_type_arc_pos_const_perm,
                       0,
                       sc_type_arc_pos_const_perm,
                       rrel_nodes);

    if (SC_TRUE == sc_iterator5_next(it))
    {
        nodes = sc_iterator5_value(it, 2);

        sc_iterator3 *nodes_it = sc_iterator3_f_a_a_new(context,
                                nodes,
                                sc_type_arc_pos_const_perm,
                                0);

        while (SC_TRUE == sc_iterator3_next(nodes_it))
        {
            sc_addr t_node = sc_iterator3_value(nodes_it, 2);

            if (SC_FALSE == find_vertex_in_set(t_node, visit))
                dfs(t_node,t_node);
        }

        sc_iterator3_free(nodes_it);
    }

    sc_iterator5_free(it);
}

void run_test(char number_test)
{
    char gr[3] = "Gx";
    gr[1] = number_test;
    sc_helper_resolve_system_identifier(context, gr, &graph);
    sc_helper_resolve_system_identifier(context, "rrel_arcs", &rrel_arcs);
    sc_helper_resolve_system_identifier(context, "rrel_nodes", &rrel_nodes);
    cout <<endl<< "Graph: ";

    print_graph();

    cout<<endl;

    CircumOfGraf();

    cout<<"CircumOfGraf = "<<MaxCycle+1<<endl;

    MaxCycle=-1;
}

int main()
{
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/egormakedon/ostis/kb.bin";
    params.config_file = "/home/egormakedon/ostis/config/sc-web.ini";
    params.ext_path = "/home/egormakedon/ostis/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    sc_memory_initialize(&params);

    context = sc_memory_context_new(sc_access_lvl_make_max);

    visit = sc_memory_node_new(context,sc_type_const);
    rrel_parent = sc_memory_node_new(context,sc_type_const);

    //////////////////////////////////////////////////////////////////////////////////
    run_test('0');
    run_test('1');
    run_test('2');
    run_test('3');
    run_test('4');
    cout <<endl<< "The end" << endl;

    sc_memory_context_free(context);

    sc_memory_shutdown(SC_TRUE);

    return 0;
}

