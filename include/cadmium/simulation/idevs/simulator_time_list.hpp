#pragma once
#include <memory_resource>   // <pmr>
#include <new>
#include <cassert>

template <typename T>
class simulator_time_ll_raw {
    struct Node {
        T      item;    ///< user value (e.g. `sim_ptr`)
        double time;    ///< next‑event time / timestamp
        Node*  next{nullptr};

        template <typename U>
        explicit Node(U&& x, double t) noexcept
            : item(std::forward<U>(x)), time(t) {}
    };

    using Alloc = std::pmr::polymorphic_allocator<Node>;

    Node* head_{nullptr};
    Node* tail_{nullptr};
    Alloc alloc_;

    /* ---- helpers ----------------------------------------------------- */
    template <typename U>
    Node* make_node(U&& v, double t) {
        Node* n = alloc_.allocate(1);
#if __cpp_lib_construct_at >= 201806L
        std::construct_at(n, std::forward<U>(v), t);
#else
        std::allocator_traits<Alloc>::construct(alloc_, n,
                                                std::forward<U>(v), t);
#endif
        return n;
    }
    void destroy_node(Node* n) noexcept {
#if __cpp_lib_destroy_at >= 201806L
        std::destroy_at(n);
#else
        std::allocator_traits<Alloc>::destroy(alloc_, n);
#endif
        alloc_.deallocate(n, 1);
    }

public:
    explicit simulator_time_ll_raw(std::pmr::memory_resource* rsrc =
                                  std::pmr::new_delete_resource()) noexcept
        : alloc_{rsrc} {}

    ~simulator_time_ll_raw() { clear(); }

    /* ---------- status ---------- */
    bool empty() const noexcept { return head_ == nullptr; }

    /* ---------- clear ----------- */
    void clear() noexcept {
        for (Node* cur = head_; cur; ) {
            Node* nxt = cur->next;
            destroy_node(cur);
            cur = nxt;
        }
        head_ = tail_ = nullptr;
    }

    /* ---------- push_front / back ---------- */
    template <typename U>
    void push_front(U&& v, double t) {
        Node* n = make_node(std::forward<U>(v), t);
        n->next = head_;
        head_   = n;
        if (!tail_) tail_ = n;        // first element
    }

    template <typename U>
    void push_back(U&& v, double t) {
        Node* n = make_node(std::forward<U>(v), t);
        if (tail_) tail_->next = n;
        else       head_       = n;   // empty list
        tail_ = n;
    }

    /* ---------- pop_front -------- */
    void pop_front() noexcept {
        if (empty()) return;
        Node* n = head_;
        head_ = head_->next;
        if (!head_) tail_ = nullptr;  // list became empty
        destroy_node(n);
    }

    /* ---------- erase_if (C++20‑style) ---------- */
    template <typename Pred>
    std::size_t erase_if(Pred&& pred) noexcept {
        std::size_t removed = 0;
        Node* prev = nullptr;
        Node* cur  = head_;
        while (cur) {
            if (pred(cur->item, cur->time)) {
                Node* doomed = cur;
                cur = cur->next;                     // advance before erase

                if (prev)  prev->next = cur;         // unlink
                else       head_      = cur;
                if (doomed == tail_) tail_ = prev;   // fix tail if needed

                destroy_node(doomed);
                ++removed;
            } else {
                prev = cur;
                cur  = cur->next;
            }
        }
        return removed;
    }

    /* ---------- traversal -------- */
    template <typename F>
    void traverse_forward(F&& fn) const {
        for (Node* cur = head_; cur; cur = cur->next)
            fn(cur->item, cur->time);
    }

    /* ---------- size (optional) -- */
    std::size_t size() const noexcept {
        std::size_t s = 0;
        for (auto p = head_; p; p = p->next) ++s;
        return s;
    }
};