#pragma once

#include <pthread.h>
#include <cassert>
#include <map>
#include <vector>
#include <memory>

#include "tamm/types.hpp"

namespace tamm {

#if 1
class ProcGroup {
 public:
  /**
   * @brief Construct a new Proc Group object
   * 
   */
  ProcGroup() : pginfo_{std::make_shared<ProcGroupInfo>()} {}
  ProcGroup(const ProcGroup&) = default;
  ProcGroup(ProcGroup&& pg) : pginfo_{std::move(pg.pginfo_)} {}
  ProcGroup& operator=(ProcGroup pg) {
    using std::swap;
    swap(*this, pg);
    return *this;
  }
  ~ProcGroup() = default;

  /**
   * @brief Construct a new Proc Group object by wrapping the given MPI
   * communicator and GA process group
   *
   * @param mpi_comm MPI communicator
   * @param ga_pg Corresponding GA process group
   *
   * @pre @param ga_pg corresponds to @param mpi_comm
   */
#if defined(USE_UPCXX)
  ProcGroup(upcxx::team* team)
#else
  ProcGroup(MPI_Comm mpi_comm, int ga_pg)
#endif
      : pginfo_{std::make_shared<ProcGroupInfo>()} {
#if defined(USE_UPCXX)
    pginfo_->team_ = team;
    pginfo_->created_team_ = false;
    pginfo_->is_valid_ = true;

    pginfo_->recvd_ops = new upcxx::dist_object<int64_t>(0, *team);

    pginfo_->pending_put_futures = upcxx::make_future();
#else
    pginfo_->mpi_comm_ = mpi_comm;
    pginfo_->created_mpi_comm_ = false;
    pginfo_->ga_pg_ = ga_pg;
    pginfo_->created_ga_pg_ = false;
    pginfo_->is_valid_ = (mpi_comm != MPI_COMM_NULL);
#endif
  }

  static ProcGroup create_world_coll() {
#if defined(USE_UPCXX)
      return create_coll(upcxx::world());
#else
      return create_coll(GA_MPI_Comm());
#endif
  }

  /**
   * @brief Collectively create a ProcGroup from the given communicator
   *
   * @param mpi_comm Communication to be used as a basis of the ProcGroup
   * @return ProcGroup New ProcGroup object that duplicates @param mpi_comm and
   * creates the corresponding GA process group
   */
#if defined(USE_UPCXX)
  static ProcGroup create_coll(upcxx::team& team) {
    return ProcGroup(&team);
  }
#else
  static ProcGroup create_coll(MPI_Comm mpi_comm) {
    MPI_Comm comm_out;
    MPI_Comm_dup(mpi_comm, &comm_out);
    ProcGroup pg;
    pg.pginfo_->mpi_comm_ = comm_out;
    pg.pginfo_->created_mpi_comm_ = true;
    pg.pginfo_->ga_pg_ = create_ga_process_group_coll(mpi_comm);
    pg.pginfo_->created_ga_pg_ = true;
    pg.pginfo_->is_valid_ = (mpi_comm != MPI_COMM_NULL);
    return pg;
  }
#endif

  /**
   * @brief Check if the given process group is valid
   * 
   * @return true if it is a non-null process group (not MPI_COMM_NULL)
   * @return false otherwise
   */
  bool is_valid() const { return pginfo_->is_valid_; }

  /**
   * Number of ranks in the wrapped communicator
   * @return Size of the wrapped communicator
   *
   * @pre is_valid()
   */
  Proc size() const {
    EXPECTS(is_valid());
#if defined(USE_UPCXX)
    return Proc{pginfo_->team_->rank_n()};
#else
    int nranks;
    MPI_Comm_size(pginfo_->mpi_comm_, &nranks);
    return Proc{nranks};
#endif
  }

#if defined(USE_UPCXX)
  /**
   * Access the underlying UPC++ team
   * @return the wrapped UPC++ team
   *
   * @pre is_valid()
   */
  upcxx::team* team() const {
    EXPECTS(is_valid());
    return pginfo_->team_;
  }
#else
  /**
   * Access the underlying MPI communicator
   * @return the wrapped MPI communicator
   *
   * @pre is_valid()
   */
  MPI_Comm comm() const {
    EXPECTS(is_valid());
    return pginfo_->mpi_comm_;
  }

  /**
   * @brief Obtained the underlying GA process group
   * 
   * @return int Underlying GA process group
   * 
   * @pre is_valid()
   */
  int ga_pg() const {
    EXPECTS(is_valid());
    return pginfo_->ga_pg_;
  }
#endif

  static Proc world_rank() {
#if defined(USE_UPCXX)
    return upcxx::rank_me();
#else
    return GA_Nodeid();
#endif
  }

  /**
   * Rank of invoking process
   * @return rank of invoking process in the wrapped communicator
   */
  Proc rank() const {
    EXPECTS(is_valid());
#if defined(USE_UPCXX)
    return pginfo_->team_->rank_me();
#else
    int rank;
    MPI_Comm_rank(pginfo_->mpi_comm_, &rank);
    return Proc{rank};
#endif
  }

  /**
   * Collectivelu clone the given process group
   * @return A copy of this process group
   */
#if defined(USE_UPCXX)
  ProcGroup clone_coll() const { return create_coll(*pginfo_->team_); }
#else
  ProcGroup clone_coll() const { return create_coll(pginfo_->mpi_comm_); }
#endif

  /**
   * @brief Collectively destroy this communicator.
   *
   * @post !is_valid()
   * @post comm() == MPI_COMM_NULL
   * @post underlying GA process group in invalid
   *
   */
  void destroy_coll() {
    EXPECTS(is_valid());
#if defined(USE_UPCXX)
    if (pginfo_->created_team_) {
      pginfo_->team_->destroy();
      pginfo_->created_team_ = false;
    }
#else
    if (pginfo_->created_mpi_comm_) {
      MPI_Comm_free(&pginfo_->mpi_comm_);
      pginfo_->created_mpi_comm_ = false;
    }
    if (pginfo_->created_ga_pg_) {
      GA_Pgroup_destroy(pginfo_->ga_pg_);
      pginfo_->created_ga_pg_ = false;
    }
#endif
    *pginfo_ = ProcGroupInfo{};  // cleanup
  }

#if defined(USE_UPCXX)
  void add_op(int rank) {
      pginfo_->sent_ops += 1;
  }

  void add_put_fut(upcxx::future<> &f) {
      pginfo_->pending_put_futures = upcxx::when_all(pginfo_->pending_put_futures, f);
  }

  upcxx::dist_object<int64_t>* get_recvd_ops_object() { return pginfo_->recvd_ops; }
#endif

  /**
   * Barrier on the wrapped communicator.
   *
   * @pre is_valid()
   */
  void barrier() {
    EXPECTS(is_valid());
#if defined(USE_UPCXX)
    pginfo_->pending_put_futures.wait();
    pginfo_->pending_put_futures = upcxx::make_future();

    int64_t total_sends;
    {
        total_sends = upcxx::reduce_all<int64_t>(pginfo_->sent_ops,
                upcxx::op_fast_add, *(pginfo_->team_)).wait();
    }

    pginfo_->sent_ops = 0;

    int64_t total_recvs;
    do {
        int64_t local_recvd = *(*(pginfo_->recvd_ops));

        {
            total_recvs = upcxx::reduce_all<int64_t>(local_recvd,
                    upcxx::op_fast_add, *(pginfo_->team_)).wait();
        }
    } while (total_recvs < total_sends);

    if (total_recvs != total_sends) {
        abort();
    }

    *(*(pginfo_->recvd_ops)) = 0;
    upcxx::barrier(*(pginfo_->team_));
#else
    GA_Pgroup_sync(pginfo_->ga_pg_);
#endif
  }

  /**
   * @brief Translate a rank from this process group to anothe process group
   * 
   * @param proc rank in this process group
   * @param pg2 Process group to be translated into
   * @return Proc rank of @param proc in ProcGroup @param pg2
   * 
   * @pre is_valid()
   * @pre proc >=0 && proc < size()
   * @pre pg2.is_valid
   * @pre proc (w.r.t. this) exists in pg2
   */
  Proc rank_translate(Proc proc, const ProcGroup& pg2) {
    EXPECTS(is_valid());
    EXPECTS(pg2.is_valid());

#if defined(USE_UPCXX)
    int this_rank = proc.value();
    int world_rank = (*pginfo_->team_)[this_rank];

    int other_rank = pg2.pginfo_->team_->from_world(world_rank, MPI_UNDEFINED);
    return Proc{other_rank};
#else
    MPI_Group group1, group2;
    int ranks1{static_cast<int>(proc.value())};
    int ranks2{MPI_PROC_NULL};
    // MPI_Comm_group(comm_, &group1);
    MPI_Comm_group(pginfo_->mpi_comm_, &group1);
    MPI_Comm_group(pg2.pginfo_->mpi_comm_, &group2);
    MPI_Group_translate_ranks(group1, 1, &ranks1, group2, &ranks2);
    assert(ranks2 != MPI_PROC_NULL);
    return Proc{ranks2};
#endif
  }

  /**
   * @brief Translate all ranks from this proc group to ranks in @param pg2 
   *
   * @param pg2 Proc group to which this group's ranks need to be translated
   * @return std::vector<Proc> Translated rank for each rank in this group. -1
   * indicates a rank in this group that is not in @param pg2
   */
  std::vector<Proc> rank_translate(const ProcGroup& pg2) {
    EXPECTS(is_valid());
    EXPECTS(pg2.is_valid());
    const size_t nranks = size().value();
    std::vector<Proc> ret(nranks);
    for(int i=0; i<nranks; i++) {
      ret[i] = rank_translate(i, pg2);
    }
    return ret;
  }

  /**
   * @brief Create a GA process group correspondig to MPI_COMM_SELF, or just
   * access it.
   *
   * @param create Create the GA pgroup if true
   * @return int Return the created GA pgroup
   */
  static int self_ga_pgroup(bool create = false) {
    static int ga_pg_self = -1;
    static bool created = false;
    if (!create) {
      EXPECTS(created);
      return ga_pg_self;
    }
    ga_pg_self = create_ga_process_group_coll(MPI_COMM_SELF);
    created = true;
    return ga_pg_self;
  }

  template <typename T>
  void broadcast(T *buf, int root) {
#if defined(USE_UPCXX)
    upcxx::broadcast(buf, 1, root, *pginfo_->team_).wait();
#else
    MPI_Bcast(buf, 1, mpi_type<T>(), root, pginfo_->mpi_comm_);
#endif
  }

  template <typename T>
  void broadcast(T *buf, int buflen, int root) {
#if defined(USE_UPCXX)
    upcxx::broadcast(buf, buflen, root, *pginfo_->team_).wait();
#else
    MPI_Bcast(buf, buflen, mpi_type<T>(), root, pginfo_->mpi_comm_);
#endif
  }

#if defined(USE_UPCXX)
  template <typename T>
  void gather(const T *sbuf, upcxx::global_ptr<T> rbuf) {
    gather(sbuf, 1, rbuf, 1);
  }

  template <typename T>
  void gather(const T *sbuf, int scount, upcxx::global_ptr<T> rbuf, int rcount) {
    EXPECTS(scount == rcount);
    EXPECTS(sbuf != nullptr || scount == 0);
    if (sbuf) {
        upcxx::rput(sbuf, rbuf + (scount * rank().value()), scount).wait();
    }
    upcxx::barrier(*pginfo_->team_);
  }

  template <typename T>
  void gatherv(const T *sbuf, int scount, upcxx::global_ptr<T> rbuf,
          const int* rcounts, upcxx::global_ptr<int> displacements) {
    upcxx::barrier(*pginfo_->team_); // ensure that displacements is populated on root
    int my_displacement = upcxx::rget(displacements + rank().value()).wait();
    EXPECTS(sbuf != nullptr || scount == 0);
    if (sbuf) {
        upcxx::rput(sbuf, rbuf + my_displacement, scount).wait();
    }
    upcxx::barrier(*pginfo_->team_); // wait for everyone to rput
  }  

  template <typename T>
  void allgather(const T *sbuf, T *rbuf) {
     throw std::runtime_error("allgather unsupported");
  }

  template <typename T>
  void allgather(const T *sbuf, int scount, T *rbuf, int rcount) {
     throw std::runtime_error("allgather unsupported");
  }
#else
  template <typename T>
  void gather(const T *sbuf, T *rbuf, int root) {
    MPI_Gather(sbuf, 1, mpi_type<T>(), rbuf, 1, mpi_type<T>(), root, pginfo_->mpi_comm_);
  }

  template <typename T>
  void gather(const T *sbuf, int scount, T *rbuf, int rcount, int root) {
    MPI_Gather(sbuf, scount, mpi_type<T>(), rbuf, rcount, mpi_type<T>(), root, pginfo_->mpi_comm_);
  }

  template <typename T>
  void gatherv(const T *sbuf, int scount, T *rbuf, const int* rcounts, const int* displacements, int root) {
    MPI_Gatherv(sbuf, scount, mpi_type<T>(), rbuf, rcounts, displacements, mpi_type<T>(), root, pginfo_->mpi_comm_);
  }

  template <typename T>
  void allgather(const T *sbuf, T *rbuf) {
    MPI_Allgather(sbuf, 1, mpi_type<T>(), rbuf, 1, mpi_type<T>(), pginfo_->mpi_comm_);
  }

  template <typename T>
  void allgather(const T *sbuf, int scount, T *rbuf, int rcount) {
    MPI_Allgather(sbuf, scount, mpi_type<T>(), rbuf, rcount, mpi_type<T>(), pginfo_->mpi_comm_);
  }
#endif

  template <typename T>
  T reduce(const T *buf, ReduceOp op, int root) {
    T result{};
#if defined(USE_UPCXX)
    if (op == ReduceOp::min) {
        upcxx::reduce_one(buf, &result, 1, upcxx::op_fast_min, root, *pginfo_->team_).wait();
    } else if (op == ReduceOp::max) {
        upcxx::reduce_one(buf, &result, 1, upcxx::op_fast_max, root, *pginfo_->team_).wait();
    } else if (op == ReduceOp::sum) {
        upcxx::reduce_one(buf, &result, 1, upcxx::op_fast_add, root, *pginfo_->team_).wait();
    } else {
        abort();
    }
#else
    MPI_Reduce(buf, &result, 1, mpi_type<T>(), mpi_op(op), root, pginfo_->mpi_comm_);
#endif
    return result;
  }

  template <typename T>
  void reduce(const T *sbuf, T *rbuf, int count, ReduceOp op, int root) {
#if defined(USE_UPCXX)
    if (op == ReduceOp::min) {
      upcxx::reduce_one(sbuf, rbuf, count, upcxx::op_fast_min, root, *pginfo_->team_).wait();
    } else if (op == ReduceOp::max) {
      upcxx::reduce_one(sbuf, rbuf, count, upcxx::op_fast_max, root, *pginfo_->team_).wait();
    } else if (op == ReduceOp::sum) {
      upcxx::reduce_one(sbuf, rbuf, count, upcxx::op_fast_add, root, *pginfo_->team_).wait();
    } else {
        abort();
    }
#else
      MPI_Reduce(sbuf, rbuf, count, mpi_type<T>(), mpi_op(op), root, pginfo_->mpi_comm_);
#endif
  }

  template <typename T>
  T allreduce(const T *buf, ReduceOp op) {
    T result{};
#if defined(USE_UPCXX)
    if (op == ReduceOp::min) {
      upcxx::reduce_all(buf, &result, 1, upcxx::op_fast_min, *pginfo_->team_).wait();
    } else if (op == ReduceOp::max) {
      upcxx::reduce_all(buf, &result, 1, upcxx::op_fast_max, *pginfo_->team_).wait();
    } else if (op == ReduceOp::sum) {
      upcxx::reduce_all(buf, &result, 1, upcxx::op_fast_add, *pginfo_->team_).wait();
    } else {
        abort();
    }
#else
    MPI_Allreduce(buf, &result, 1, mpi_type<T>(), mpi_op(op), pginfo_->mpi_comm_);
#endif
    return result;
  }

  template <typename T>
  void allreduce(const T *sbuf, T *rbuf, int count, ReduceOp op) {
#if defined(USE_UPCXX)

    if (op == ReduceOp::min) {
      upcxx::reduce_all(sbuf, rbuf, count, upcxx::op_fast_min, *pginfo_->team_).wait();
    } else if (op == ReduceOp::max) {
      upcxx::reduce_all(sbuf, rbuf, count, upcxx::op_fast_max, *pginfo_->team_).wait();
    } else if (op == ReduceOp::sum) {
      upcxx::reduce_all(sbuf, rbuf, count, upcxx::op_fast_add, *pginfo_->team_).wait();
    } else {
        abort();
    }
#else
    MPI_Allreduce(sbuf, rbuf, count, mpi_type<T>(), mpi_op(op), pginfo_->mpi_comm_);
#endif
  }

 private:
  /**
   * Create a GA process group corresponding to the given proc group
   * @param pg TAMM process group
   * @return GA processes group on this TAMM process group
   */
  static int create_ga_process_group_coll(MPI_Comm comm) {
    int nranks;
    MPI_Comm_size(comm, &nranks);
    MPI_Group group, group_world;
    int ranks[nranks], ranks_world[nranks];
    MPI_Comm_group(comm, &group);

    MPI_Comm_group(GA_MPI_Comm(), &group_world);

    for (int i = 0; i < nranks; i++) {
      ranks[i] = i;
    }
    MPI_Group_translate_ranks(group, nranks, ranks, group_world, ranks_world);

    int ga_pg_default = GA_Pgroup_get_default();
    GA_Pgroup_set_default(GA_Pgroup_get_world());
    int ga_pg = GA_Pgroup_create(ranks_world, nranks);
    GA_Pgroup_set_default(ga_pg_default);
    return ga_pg;
  }

  /**
   * @brief Swap contents of two given objects
   * 
   * @param first Object to be swapped
   * @param second Object to be swapped
   */
  friend void swap(ProcGroup& first, ProcGroup& second) {
    using std::swap;
    swap(first.pginfo_, second.pginfo_);
  }

  /**
   * @brief Object holding ProcGroup's shared state. It trackes whether the MPI
   * communicator and GA pgroup were created/dup-ed and this have to be
   * destructed.
   *
   */
  struct ProcGroupInfo {
#if defined(USE_UPCXX)
    upcxx::team* team_ = NULL; /**< Wrapped MPI communicator */
    bool created_team_ = false;     /**< Was mpi_comm_ duplicated/created */
#else
    MPI_Comm mpi_comm_ = MPI_COMM_NULL; /**< Wrapped MPI communicator */
    bool created_mpi_comm_ = false;     /**< Was mpi_comm_ duplicated/created */
    int ga_pg_ = -1;                    /**< Corresponding GA communicator */
    bool created_ga_pg_ = false;        /**< Was this GA pgroup created */
#endif
    bool is_valid_ =
        false; /**< Is this object valid, i.e., mpi_comm_ != MPI_COMM_NULL */
#if defined(USE_UPCXX)
    int64_t sent_ops = 0;
    upcxx::dist_object<int64_t> *recvd_ops = NULL;
    upcxx::future<> pending_put_futures;
#endif
  };
  std::shared_ptr<ProcGroupInfo> pginfo_; /**< Shared ProcGroupInfo state */

  /**
   * @brief Equality operator
   * 
   * @param lhs LHS to be compared
   * @param rhs RHS to be compared
   * @return true if lhs == rhs
   * @return false otherwise
   */
  friend bool operator==(const ProcGroup& lhs, const ProcGroup& rhs) {
#if defined(USE_UPCXX)
    return lhs.pginfo_->team_->id() == rhs.pginfo_->team_->id();
#else
    int result;
    MPI_Comm_compare(lhs.pginfo_->mpi_comm_, rhs.pginfo_->mpi_comm_, &result);
    return result == MPI_IDENT;
#endif
  }

  /**
   * @brief Inequality operator
   * 
   * @param lhs LHS to be compared
   * @param rhs RHS to be compared
   * @return true if lhs != rhs
   * @return false ptherwise
   */
  friend bool operator!=(const ProcGroup& lhs, const ProcGroup& rhs) {
    return !(lhs == rhs);
  }
};  // class ProcGroup

#else
/**
 * @brief Wrapper to MPI communicator and related operations.
 */
class ProcGroup {
 public:
  //ProcGroup() = default;
  ProcGroup() : mpi_comm_(std::make_shared<MPI_Comm>(MPI_COMM_NULL)), is_valid_(false){
    // MPI_Comm* comm_out = new MPI_Comm();
    // *comm_out = MPI_COMM_NULL;
    // mpi_comm_.reset(comm_out);
  }
  ProcGroup(MPI_Comm comm) {
    //std::shared_ptr<MPI_Comm> comm_out = std::make_shared<MPI_Comm> (comm);
    // mpi_comm_.reset(comm_out.get());
    //MPI_Comm* comm_out = new MPI_Comm();
    //*comm_out = comm;
    //mpi_comm_.reset(comm_out);
    mpi_comm_.reset(new MPI_Comm(comm));
    is_valid_=(comm != MPI_COMM_NULL);
  }
  static ProcGroup create_coll(MPI_Comm mpi_comm) {
    MPI_Comm* comm_out = new MPI_Comm();
    MPI_Comm_dup(mpi_comm, comm_out);
    ProcGroup pg;
    pg.mpi_comm_.reset(comm_out, deleter);
    pg.is_valid_ = true;
    pg.ga_pg_ = create_ga_process_group_coll(mpi_comm);
    return pg;
  }

  ProcGroup(const ProcGroup&) = default;
  ProcGroup(ProcGroup&& pg)  // TBD: check if this can be default
      : mpi_comm_{std::move(pg.mpi_comm_)},
      ga_pg_{pg.ga_pg_} {}

  ProcGroup& operator=(const ProcGroup&) = default;
  //ProcGroup(ProcGroup&&) = default;
  ProcGroup& operator=(ProcGroup&&) = default;
  ~ProcGroup() = default;

  //explicit ProcGroup(MPI_Comm comm = MPI_COMM_NULL)
  //    : comm_{comm},
  //      is_valid_{comm != MPI_COMM_NULL} { }

  /**
   * Is it a valid communicator (i.e., not MPI_COMM_NULL)
   * @return true is wrapped MPI communicator is not MPI_COMM_NULL
   */
  bool is_valid() const {
    return is_valid_;
  }

  /**
   * Rank of invoking process
   * @return rank of invoking process in the wrapped communicator
   */
  Proc rank() const {
    int rank;
    EXPECTS(is_valid());
    //MPI_Comm_rank(comm_, &rank);
    MPI_Comm_rank(*mpi_comm_, &rank);
    return Proc{rank};
  }

  /**
   * Number of ranks in the wrapped communicator
   * @return Size of the wrapped communicator
   */
  Proc size() const {
    int nranks;
    EXPECTS(is_valid());
    //MPI_Comm_size(comm_, &nranks);
    MPI_Comm_size(*mpi_comm_, &nranks);
    return Proc{nranks};
  }

  /**
   * Access the underlying MPI communicator
   * @return the wrapped MPI communicator
   */
  MPI_Comm comm() const {
    //return comm_;
    return *mpi_comm_;
  }

  int ga_pg() const {
    return ga_pg_;
  }

  /**
   * Duplicate/clone the wrapped MPI communicator
   * @return A copy.
   * @note This is a collective call on the wrapped communicator
   * @todo Rename this call to clone_coll() to indicate this is a collective call.
   */
  ProcGroup clone_coll() const { return create_coll(*mpi_comm_); }
  
  //ProcGroup clone() const {
  //  EXPECTS(is_valid());
  //  MPI_Comm comm_out{MPI_COMM_NULL};
  //  MPI_Comm_dup(comm_, &comm_out);
  //  return ProcGroup{comm_out};
  //}

void destroy_coll() { 
  MPI_Comm_free(mpi_comm_.get());
  GA_Pgroup_destroy(ga_pg_);
  is_valid_ = false;
  }
  /**
   * Free the wrapped communicator
   */
 /* void destroy() {
    if(is_valid()) {
      MPI_Comm_free(&comm_);
    }
    comm_ = MPI_COMM_NULL;
    is_valid_ = false;
  }*/

  /**
   * Barrier on the wrapped communicator.
   */
  void barrier() {
    //MPI_Barrier(comm_);
    //MPI_Barrier(*mpi_comm_);
    GA_Pgroup_sync(ga_pg_);
  }
  
  Proc rank_translate(Proc proc, const ProcGroup& pg2) {
    EXPECTS(is_valid());
    MPI_Group group1, group2;
    int ranks1{static_cast<int>(proc.value())};
    int ranks2{MPI_PROC_NULL};
    //MPI_Comm_group(comm_, &group1);
    MPI_Comm_group(*mpi_comm_, &group1);
    MPI_Comm_group(*pg2.mpi_comm_, &group2);
    MPI_Group_translate_ranks(group1, 1, &ranks1, group2, &ranks2);
    assert(ranks2 != MPI_PROC_NULL);
    return Proc{ranks2};
  }
  

 private:
  /**
   * Create a GA process group corresponding to the given proc group
   * @param pg TAMM process group
   * @return GA processes group on this TAMM process group
   */
  static int create_ga_process_group_coll(MPI_Comm comm) {
    int nranks;
    MPI_Comm_size(comm, &nranks);
    MPI_Group group, group_world;
    int ranks[nranks], ranks_world[nranks];
    MPI_Comm_group(comm, &group);
    MPI_Comm_group(GA_MPI_Comm(), &group_world);

    for (int i = 0; i < nranks; i++) {
      ranks[i] = i;
    }
    MPI_Group_translate_ranks(group, nranks, ranks, group_world, ranks_world);

    int ga_pg_default = GA_Pgroup_get_default();
    GA_Pgroup_set_default(GA_Pgroup_get_world());
    int ga_pg = GA_Pgroup_create(ranks_world, nranks);
    GA_Pgroup_set_default(ga_pg_default);
    return ga_pg;
  }

  // MPI_Comm comm_;// = MPI_COMM_NULL;
  std::shared_ptr<MPI_Comm> mpi_comm_;
  int ga_pg_;
  bool is_valid_;

  static void deleter(MPI_Comm* mpi_comm) {
    EXPECTS(*mpi_comm != MPI_COMM_NULL);
    delete mpi_comm;
  }

  static void deleter_comm(MPI_Comm* mpi_comm) {
    delete mpi_comm;
  }

  friend bool operator == (const ProcGroup& lhs, const ProcGroup& rhs) {
    int result;
    MPI_Comm_compare(*lhs.mpi_comm_, *rhs.mpi_comm_, &result);
    return result == MPI_IDENT;
  }
  
  friend  bool operator != (const ProcGroup& lhs, const ProcGroup& rhs) {
    return !(lhs == rhs);
  }
};  // class ProcGroup
#endif

}  // namespace tamm
