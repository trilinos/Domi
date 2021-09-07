// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#include "Kokkos_Core.hpp"            // for parallel_for, KOKKOS_LAMBDA
#include "gtest/gtest.h"              // for AssertionResult, Message, TestPartResult, EXPECT_EQ
#include "stk_ngp_test/ngp_test.hpp"  // for NGP_EXPECT_EQ, NGP_EXPECT_FALSE, NGP_EXPECT_TRUE
#include "stk_topology/topology.hpp"  // for topology, topology::EDGE_RANK, topology::FACE_RANK
#include "topology_test_utils.hpp"    // for check_edge_node_ordinals, check_edge_node_ordinals_ngp
#include <vector>                     // for vector

TEST( stk_topology, tri_3)
{
  using stk::topology;

  topology t = topology::TRI_3;

  EXPECT_TRUE(t.is_valid());
  EXPECT_FALSE(t.has_homogeneous_faces());
  EXPECT_FALSE(t.is_shell());

  EXPECT_EQ(t.rank(),topology::FACE_RANK);
  EXPECT_EQ(t.side_rank(),topology::EDGE_RANK);
  EXPECT_EQ(t.num_sides(),3u);

  EXPECT_EQ(t.dimension(),2u);
  EXPECT_EQ(t.num_nodes(),3u);
  EXPECT_EQ(t.num_vertices(),3u);
  EXPECT_EQ(t.num_edges(),3u);
  EXPECT_EQ(t.num_faces(),0u);
  EXPECT_EQ(t.num_permutations(),6u);
  EXPECT_EQ(t.num_positive_permutations(),3u);

  EXPECT_FALSE(t.defined_on_spatial_dimension(1));
  EXPECT_FALSE(t.defined_on_spatial_dimension(2));
  EXPECT_TRUE(t.defined_on_spatial_dimension(3));

  EXPECT_EQ(t.base(),topology::TRI_3);

  EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

  std::vector<std::vector<unsigned>> gold_edge_node_ordinals = { {0, 1},
                                                                 {1, 2},
                                                                 {2, 0} };
  check_side_node_ordinals(t, gold_edge_node_ordinals);
  check_edge_node_ordinals(t, gold_edge_node_ordinals);
  check_side_nodes(t, gold_edge_node_ordinals);
  check_edge_nodes(t, gold_edge_node_ordinals);

  std::vector<std::vector<unsigned>> gold_permutation_node_ordinals = { {0, 1, 2},
                                                                        {2, 0, 1},
                                                                        {1, 2, 0},
                                                                        {0, 2, 1},
                                                                        {2, 1, 0},
                                                                        {1, 0, 2} };
  check_permutation_node_ordinals(t, gold_permutation_node_ordinals);
  check_permutation_nodes(t, gold_permutation_node_ordinals);

  check_equivalent(t, gold_permutation_node_ordinals);
  check_lexicographical_smallest_permutation(t, gold_permutation_node_ordinals);
}

void check_tri3_on_device()
{
  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int i)
  {
    stk::topology t = stk::topology::TRI_3;

    NGP_EXPECT_TRUE(t.is_valid());
    NGP_EXPECT_FALSE(t.has_homogeneous_faces());
    NGP_EXPECT_FALSE(t.is_shell());

    NGP_EXPECT_EQ(t.rank(),stk::topology::FACE_RANK);
    NGP_EXPECT_EQ(t.side_rank(),stk::topology::EDGE_RANK);
    NGP_EXPECT_EQ(t.num_sides(),3u);

    NGP_EXPECT_EQ(t.dimension(),2u);
    NGP_EXPECT_EQ(t.num_nodes(),3u);
    NGP_EXPECT_EQ(t.num_vertices(),3u);
    NGP_EXPECT_EQ(t.num_edges(),3u);
    NGP_EXPECT_EQ(t.num_faces(),0u);
    NGP_EXPECT_EQ(t.num_permutations(),6u);
    NGP_EXPECT_EQ(t.num_positive_permutations(),3u);

    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(1));
    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(2));
    NGP_EXPECT_TRUE(t.defined_on_spatial_dimension(3));

    NGP_EXPECT_EQ(t.base(),stk::topology::TRI_3);

    NGP_EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

    unsigned gold_edge_node_ordinals[3][2] = { {0, 1},
                                               {1, 2},
                                               {2, 0} };
    check_side_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_edge_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_side_nodes_ngp(t, gold_edge_node_ordinals);
    check_edge_nodes_ngp(t, gold_edge_node_ordinals);

    unsigned gold_permutation_node_ordinals[6][3] = { {0, 1, 2},
                                                      {2, 0, 1},
                                                      {1, 2, 0},
                                                      {0, 2, 1},
                                                      {2, 1, 0},
                                                      {1, 0, 2} };
    check_permutation_node_ordinals_ngp(t, gold_permutation_node_ordinals);
    check_permutation_nodes_ngp(t, gold_permutation_node_ordinals);

    check_equivalent_ngp(t, gold_permutation_node_ordinals);
    check_lexicographical_smallest_permutation_ngp(t, gold_permutation_node_ordinals);
  });
}

NGP_TEST(stk_topology_ngp, tri_3)
{
  check_tri3_on_device();
}

TEST( stk_topology, tri_4)
{
  using stk::topology;

  topology t = topology::TRI_4;

  EXPECT_TRUE(t.is_valid());
  EXPECT_FALSE(t.has_homogeneous_faces());
  EXPECT_FALSE(t.is_shell());

  EXPECT_EQ(t.rank(),topology::FACE_RANK);
  EXPECT_EQ(t.side_rank(),topology::EDGE_RANK);
  EXPECT_EQ(t.num_sides(),3u);

  EXPECT_EQ(t.dimension(),2u);
  EXPECT_EQ(t.num_nodes(),4u);
  EXPECT_EQ(t.num_vertices(),3u);
  EXPECT_EQ(t.num_edges(),3u);
  EXPECT_EQ(t.num_faces(),0u);
  EXPECT_EQ(t.num_permutations(),6u);
  EXPECT_EQ(t.num_positive_permutations(),3u);

  EXPECT_FALSE(t.defined_on_spatial_dimension(1));
  EXPECT_FALSE(t.defined_on_spatial_dimension(2));
  EXPECT_TRUE(t.defined_on_spatial_dimension(3));

  EXPECT_EQ(t.base(),topology::TRI_3);

  EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

  std::vector<std::vector<unsigned>> gold_edge_node_ordinals = { {0, 1},
                                                                 {1, 2},
                                                                 {2, 0} };
  check_side_node_ordinals(t, gold_edge_node_ordinals);
  check_edge_node_ordinals(t, gold_edge_node_ordinals);
  check_side_nodes(t, gold_edge_node_ordinals);
  check_edge_nodes(t, gold_edge_node_ordinals);

  std::vector<std::vector<unsigned>> gold_permutation_node_ordinals = { {0, 1, 2, 3},
                                                                        {2, 0, 1, 3},
                                                                        {1, 2, 0, 3},
                                                                        {0, 2, 1, 3},
                                                                        {2, 1, 0, 3},
                                                                        {1, 0, 2, 3} };
  check_permutation_node_ordinals(t, gold_permutation_node_ordinals);
  check_permutation_nodes(t, gold_permutation_node_ordinals);

  check_equivalent(t, gold_permutation_node_ordinals);
  check_lexicographical_smallest_permutation(t, gold_permutation_node_ordinals);
}

void check_tri4_on_device()
{
  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int i)
  {
    stk::topology t = stk::topology::TRI_4;

    NGP_EXPECT_TRUE(t.is_valid());
    NGP_EXPECT_FALSE(t.has_homogeneous_faces());
    NGP_EXPECT_FALSE(t.is_shell());

    NGP_EXPECT_EQ(t.rank(),stk::topology::FACE_RANK);
    NGP_EXPECT_EQ(t.side_rank(),stk::topology::EDGE_RANK);
    NGP_EXPECT_EQ(t.num_sides(),3u);

    NGP_EXPECT_EQ(t.dimension(),2u);
    NGP_EXPECT_EQ(t.num_nodes(),4u);
    NGP_EXPECT_EQ(t.num_vertices(),3u);
    NGP_EXPECT_EQ(t.num_edges(),3u);
    NGP_EXPECT_EQ(t.num_faces(),0u);
    NGP_EXPECT_EQ(t.num_permutations(),6u);
    NGP_EXPECT_EQ(t.num_positive_permutations(),3u);

    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(1));
    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(2));
    NGP_EXPECT_TRUE(t.defined_on_spatial_dimension(3));

    NGP_EXPECT_EQ(t.base(),stk::topology::TRI_3);

    NGP_EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

    unsigned gold_edge_node_ordinals[3][2] = { {0, 1},
                                               {1, 2},
                                               {2, 0} };
    check_side_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_edge_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_side_nodes_ngp(t, gold_edge_node_ordinals);
    check_edge_nodes_ngp(t, gold_edge_node_ordinals);

    unsigned gold_permutation_node_ordinals[6][4] = { {0, 1, 2, 3},
                                                      {2, 0, 1, 3},
                                                      {1, 2, 0, 3},
                                                      {0, 2, 1, 3},
                                                      {2, 1, 0, 3},
                                                      {1, 0, 2, 3} };
    check_permutation_node_ordinals_ngp(t, gold_permutation_node_ordinals);
    check_permutation_nodes_ngp(t, gold_permutation_node_ordinals);

    check_equivalent_ngp(t, gold_permutation_node_ordinals);
    check_lexicographical_smallest_permutation_ngp(t, gold_permutation_node_ordinals);
  });
}

NGP_TEST(stk_topology_ngp, tri_4)
{
  check_tri4_on_device();
}

TEST( stk_topology, tri_6)
{
  using stk::topology;

  topology t = topology::TRI_6;

  EXPECT_TRUE(t.is_valid());
  EXPECT_FALSE(t.has_homogeneous_faces());
  EXPECT_FALSE(t.is_shell());

  EXPECT_EQ(t.rank(),topology::FACE_RANK);
  EXPECT_EQ(t.side_rank(),topology::EDGE_RANK);
  EXPECT_EQ(t.num_sides(),3u);

  EXPECT_EQ(t.dimension(),2u);
  EXPECT_EQ(t.num_nodes(),6u);
  EXPECT_EQ(t.num_vertices(),3u);
  EXPECT_EQ(t.num_edges(),3u);
  EXPECT_EQ(t.num_faces(),0u);
  EXPECT_EQ(t.num_permutations(),6u);
  EXPECT_EQ(t.num_positive_permutations(),3u);

  EXPECT_FALSE(t.defined_on_spatial_dimension(1));
  EXPECT_FALSE(t.defined_on_spatial_dimension(2));
  EXPECT_TRUE(t.defined_on_spatial_dimension(3));

  EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

  std::vector<std::vector<unsigned>> gold_edge_node_ordinals = { {0, 1, 3},
                                                                 {1, 2, 4},
                                                                 {2, 0, 5} };
  check_side_node_ordinals(t, gold_edge_node_ordinals);
  check_edge_node_ordinals(t, gold_edge_node_ordinals);
  check_side_nodes(t, gold_edge_node_ordinals);
  check_edge_nodes(t, gold_edge_node_ordinals);

  std::vector<std::vector<unsigned>> gold_permutation_node_ordinals = { {0, 1, 2, 3, 4, 5},
                                                                        {2, 0, 1, 5, 3, 4},
                                                                        {1, 2, 0, 4, 5, 3},
                                                                        {0, 2, 1, 5, 4, 3},
                                                                        {2, 1, 0, 4, 3, 5},
                                                                        {1, 0, 2, 3, 5, 4} };
  check_permutation_node_ordinals(t, gold_permutation_node_ordinals);
  check_permutation_nodes(t, gold_permutation_node_ordinals);

  check_equivalent(t, gold_permutation_node_ordinals);
  check_lexicographical_smallest_permutation(t, gold_permutation_node_ordinals);
}

void check_tri6_on_device()
{
  Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int i)
  {
    stk::topology t = stk::topology::TRI_6;

    NGP_EXPECT_TRUE(t.is_valid());
    NGP_EXPECT_FALSE(t.has_homogeneous_faces());
    NGP_EXPECT_FALSE(t.is_shell());

    NGP_EXPECT_EQ(t.rank(),stk::topology::FACE_RANK);
    NGP_EXPECT_EQ(t.side_rank(),stk::topology::EDGE_RANK);
    NGP_EXPECT_EQ(t.num_sides(),3u);

    NGP_EXPECT_EQ(t.dimension(),2u);
    NGP_EXPECT_EQ(t.num_nodes(),6u);
    NGP_EXPECT_EQ(t.num_vertices(),3u);
    NGP_EXPECT_EQ(t.num_edges(),3u);
    NGP_EXPECT_EQ(t.num_faces(),0u);
    NGP_EXPECT_EQ(t.num_permutations(),6u);
    NGP_EXPECT_EQ(t.num_positive_permutations(),3u);

    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(1));
    NGP_EXPECT_FALSE(t.defined_on_spatial_dimension(2));
    NGP_EXPECT_TRUE(t.defined_on_spatial_dimension(3));

    NGP_EXPECT_EQ(t.face_topology(0), stk::topology::INVALID_TOPOLOGY);

    unsigned gold_edge_node_ordinals[3][3] = { {0, 1, 3},
                                               {1, 2, 4},
                                               {2, 0, 5} };
    check_side_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_edge_node_ordinals_ngp(t, gold_edge_node_ordinals);
    check_side_nodes_ngp(t, gold_edge_node_ordinals);
    check_edge_nodes_ngp(t, gold_edge_node_ordinals);

    unsigned gold_permutation_node_ordinals[6][6] = { {0, 1, 2, 3, 4, 5},
                                                      {2, 0, 1, 5, 3, 4},
                                                      {1, 2, 0, 4, 5, 3},
                                                      {0, 2, 1, 5, 4, 3},
                                                      {2, 1, 0, 4, 3, 5},
                                                      {1, 0, 2, 3, 5, 4} };
    check_permutation_node_ordinals_ngp(t, gold_permutation_node_ordinals);
    check_permutation_nodes_ngp(t, gold_permutation_node_ordinals);

    check_equivalent_ngp(t, gold_permutation_node_ordinals);
    check_lexicographical_smallest_permutation_ngp(t, gold_permutation_node_ordinals);
  });
}

NGP_TEST(stk_topology_ngp, tri_6)
{
  check_tri6_on_device();
}
