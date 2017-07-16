#ifndef TAMMX_PROC_GROUP_H_
#define TAMMX_PROC_GROUP_H_

#include <mpi.h>
#include <pthread.h>
#include <cassert>
#include <map>
#include <vector>

#include "tammx/types.h"

namespace tammx {

class ProcGroup {
 public:
  ProcGroup() = default;
  ProcGroup(const ProcGroup&) = default;
  ProcGroup& operator = (const ProcGroup&) = default;
  ~ProcGroup() = default;

  explicit ProcGroup(const MPI_Comm& comm) : comm_{comm} {}

  Proc rank() const {
    int rank;
    MPI_Comm_rank(comm_, &rank);
    return Proc{rank};
  }

  Proc size() const {
    int nranks;
    MPI_Comm_size(comm_, &nranks);
    return Proc{nranks};
  }
  
  MPI_Comm comm() const {
    return comm_;
  }

  ProcGroup clone() const {
    MPI_Comm comm_out{MPI_COMM_NULL};
    MPI_Comm_dup(comm_, &comm_out);
    return ProcGroup{comm_out};
  }
  
  void destroy() {
    MPI_Comm_free(&comm_);
    comm_ = MPI_COMM_NULL;
  }
  
  Proc rank_translate(Proc proc, const ProcGroup& pg2) {
    MPI_Group group1, group2;
    int ranks1{proc.value()};
    int ranks2{MPI_PROC_NULL};
    MPI_Comm_group(comm_, &group1);
    MPI_Comm_group(pg2.comm_, &group2);
    MPI_Group_translate_ranks(group1, 1, &ranks1, group2, &ranks2);
    assert(ranks2 != MPI_PROC_NULL);
    return Proc{ranks2};
  }
  

 private:
  MPI_Comm comm_ = MPI_COMM_NULL;

  friend bool operator == (const ProcGroup& lhs, const ProcGroup& rhs) {
    int result;
    MPI_Comm_compare(lhs.comm_, rhs.comm_, &result);
    return result == MPI_IDENT;
  }
  
  friend  bool operator != (const ProcGroup& lhs, const ProcGroup& rhs) {
    return !(lhs == rhs);
  }  
};  // class ProcGroup



} // namespace tammx


#endif // TAMMX_PROC_GROUP_H_

