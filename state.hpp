#ifndef FSM11_STATE_HPP
#define FSM11_STATE_HPP

#include <statemachine_fwd.hpp>
#include <stringref.hpp>

#include <atomic>
#include <cassert>
#include <iterator>

namespace fsm11
{

//! \brief A state in a state machine.
//!
//! The State defines a state in a finite state machine.
template <typename TOptions>
class State
{
public:
    using event_type = typename TOptions::event_type;
    using state_machine_type = fsm11_detail::StateMachine<TOptions>;
    using transition_type = Transition<TOptions>;

    //! \brief The possible child modes.
    //!
    //! This enum lists the possible child modes.
    //! - Exclusive: The children are active exclusively (i.e. one and only
    //!   one child will be active).
    //! - Parallel: The children are active parallely (i.e. all children
    //!   will be active simultanously).
    enum ChildMode
    {
        Exclusive,
        Parallel
    };


    //! \brief Constructs a state.
    //!
    //! Constructs a state with given \p name which will be a child of the
    //! \p parent state. The \p parent may be a null-pointer. In this case
    //! the state is at the root of its hierarchy.
    explicit State(const char* name, State* parent = 0);

    //! \brief Destroys the state.
    virtual ~State() {}

    State(const State&) = delete;
    State& operator=(const State&) = delete;

    //! \brief Returns the child mode.
    //!
    //! Returns the current child mode. The default is Exclusive.
    ChildMode childMode() const
    {
        return m_childMode;
    }

    State* findChild(StringRef name) const;

    //! \brief Checks if the state is active.
    //!
    //! Returns \p true, if the state is active, which means that it belongs
    //! to the current state machine configuration.
    bool isActive() const;

    //! \brief Checks for atomicity.
    //!
    //! Returns \p true, if this state is atomic which means that it does not
    //! have children.
    bool isAtomic() const
    {
        return m_children == 0;
    }

    //! \brief Checks for a compound state.
    //!
    //! Returns \p true, if this state is a compound state, i.e. it does
    //! have at least one child and the children are active exclusively.
    //! One and only one child of an active compound state will be active.
    bool isCompound() const
    {
        return !isAtomic() && m_childMode == Exclusive;
    }

    //! \brief Checks for a parallel state.
    //!
    //! Returns \p true, if this state is a parallel state, i.e. it does
    //! have at least one child and the children are active in parallel.
    //! All children of an active parallel state will be active.
    bool isParallel() const
    {
        return !isAtomic() && m_childMode == Parallel;
    }

    //! \brief The name.
    //!
    //! Returns the state's name.
    const char* name() const
    {
        return m_name;
    }

    //! \brief The on-entry method.
    //!
    //! This method is called by the state machine, whenever this state
    //! is entered. The event which triggered the configuration change
    //! is passed in \p event.
    virtual void onEntry(event_type /*event*/)
    {
        // The default implementation does nothing.
    }

    //! \brief The on-exit method.
    //!
    //! This method is called by the state machine, when the staet is left.
    //! The event which triggered the configuration change is passed in
    //! \p event.
    virtual void onExit(event_type /*event*/)
    {
        // The default implementation does nothing.
    }

    virtual void enterInvoke()
    {
        // The default implementation does nothing.
    }

    virtual void exitInvoke()
    {
        // The default implementation does nothing.
    }

    //! \brief The parent.
    //!
    //! Returns the state's parent or a null-pointer, if the state has no
    //! parent.
    State* parent() const
    {
        return m_parent;
    }

    //! \brief Sets the child mode.
    //!
    //! Sets the child mode to \p mode. Setting the child mode to Exclusive,
    //! will turn the state into a compound state. When the child mode is
    //! set to Parallel, this state will become a parallel state.
    //!
    //! \note The state machine's configuration won't be updated, if the child
    //! mode of one of its states changes. Changing the child mode while
    //! the associated state machine is running, results in undefined
    //! behaviour.
    void setChildMode(ChildMode mode)
    {
        m_childMode = mode;
    }

    //! \brief Changes the parent.
    //!
    //! Removes this child from its old parent and makes it a child of the
    //! new \p parent state. If \p parent is a null-pointer, this state will
    //! be at the root of its hieararchy.
    //!
    //! \note Changing the parent state while the associated state machine
    //! is running results in undefined behaviour.
    void setParent(State* parent);

    //! \brief Returns the state machine.
    //!
    //! Returns the state machine to which this state belongs. If the state
    //! has not been added to a state machine, yet, a null-pointer is
    //! returned.
    state_machine_type* stateMachine() const
    {
        return m_stateMachine;
    }

    // -------------------------------------------------------------------------
    // State iterators
    // -------------------------------------------------------------------------

    template <bool TIsConst>
    class PreOrderIterator;
    typedef PreOrderIterator<false> pre_order_iterator;
    typedef PreOrderIterator<true>  const_pre_order_iterator;

    template <bool TIsConst>
    class PreOrderView;

    template <bool TIsConst>
    class PostOrderIterator;
    typedef PostOrderIterator<false> post_order_iterator;
    typedef PostOrderIterator<true>  const_post_order_iterator;

    template <bool TIsConst>
    class PostOrderView;

    template <bool TIsConst>
    class SiblingIterator;
    typedef SiblingIterator<false> sibling_iterator;
    typedef SiblingIterator<true> const_sibling_iterator;

    template <bool TIsConst>
    class SiblingView;


    //! \brief A pre-order iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_begin().
    pre_order_iterator begin()
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_begin().
    const_pre_order_iterator begin() const
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_cbegin().
    const_pre_order_iterator cbegin() const
    {
        return pre_order_cbegin();
    }

    //! \brief A pre-order iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_end().
    pre_order_iterator end()
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_end().
    const_pre_order_iterator end() const
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_cend().
    const_pre_order_iterator cend() const
    {
        return pre_order_cend();
    }

    //! \brief A pre-order iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order iterator to the first state of the sub-tree
    //! rooted at this state.
    pre_order_iterator pre_order_begin() noexcept
    {
        return pre_order_iterator(this);
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state.
    const_pre_order_iterator pre_order_begin() const noexcept
    {
        return const_pre_order_iterator(this);
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state.
    const_pre_order_iterator pre_order_cbegin() const noexcept
    {
        return const_pre_order_iterator(this);
    }

    //! \brief A pre-order iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order iterator past the last state of the sub-tree
    //! rooted at this state.
    pre_order_iterator pre_order_end() noexcept
    {
        pre_order_iterator iter(this);
        iter.skipChildren();
        return ++iter;
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state.
    const_pre_order_iterator pre_order_end() const noexcept
    {
        const_pre_order_iterator iter(this);
        iter.skipChildren();
        return ++iter;
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state..
    const_pre_order_iterator pre_order_cend() const noexcept
    {
        const_pre_order_iterator iter(this);
        iter.skipChildren();
        return ++iter;
    }

    //! \brief Returns a pre-order sub-tree.
    //!
    //! Returns a helper object, which enables to use pre-order iterators over
    //! the sub-tree in a range-based for loop.
    //! \code
    //! State* root;
    //! for (auto& state : root->pre_order_subtree())
    //! {
    //!     // state accesses all descendants of root in pre-order.
    //! }
    //! \endcode
    PreOrderView<false> pre_order_subtree() noexcept
    {
        return PreOrderView<false>(this);
    }

    PreOrderView<true> pre_order_subtree() const noexcept
    {
        return PreOrderView<true>(this);
    }


    //! \brief A post-order iterator to the first state of the sub-tree.
    //!
    //! Returns a post-order iterator to the first state of the sub-tree
    //! rooted at this state..
    post_order_iterator post_order_begin() noexcept
    {
        State* state = this;
        while (state->m_children)
            state = state->m_children;
        return post_order_iterator(state);
    }

    //! \brief A post-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a post-order const-iterator to the first state of the sub-tree
    //! rooted at this state..
    const_post_order_iterator post_order_begin() const noexcept
    {
        return post_order_cbegin();
    }

    //! \brief A post-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a post-order const-iterator to the first state of the sub-tree
    //! rooted at this state.
    const_post_order_iterator post_order_cbegin() const noexcept
    {
        return const_cast<State*>(this)->post_order_begin();
    }

    //! \brief A post-order iterator past the last state of the sub-tree.
    //!
    //! Returns a post-order iterator past the last state of the sub-tree
    //! rooted at this state.
    post_order_iterator post_order_end() noexcept
    {
        return ++post_order_iterator(this);
    }

    //! \brief A post-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a post-order const-iterator past the last state of the sub-tree
    //! rooted at this state.
    const_post_order_iterator post_order_end() const noexcept
    {
        return ++const_post_order_iterator(this);
    }

    //! \brief A post-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a post-order const-iterator past the last state of the sub-tree
    //! rooted at this state.
    const_post_order_iterator post_order_cend() const noexcept
    {
        return ++const_post_order_iterator(this);
    }

    PostOrderView<false> post_order_subtree() noexcept
    {
        return PostOrderView<false>(this);
    }

    PostOrderView<true> post_order_subtree() const noexcept
    {
        return PostOrderView<true>(this);
    }


    //! \brief A pre-order iterator.
    //!
    //! The PreOrderIterator is a pre-order depth-first iterator over a
    //! hierarchy of states. If \p TIsConst is set, it implements
    //! a const-iterator, otherwise the accessed state is mutable.
    template <bool TIsConst>
    class PreOrderIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef State value_type;
        typedef typename std::conditional<TIsConst, const State*, State*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const State&, State&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        PreOrderIterator() noexcept
            : m_current(0),
              m_skipChildren(false)
        {
        }

        //! (Copy-)Constructs an iterator from a non-const iterator.
        PreOrderIterator(const PreOrderIterator<false>& other) noexcept
            : m_current(other.m_current),
              m_skipChildren(false)
        {
        }

        //! Prefix increment.
        PreOrderIterator& operator++() noexcept
        {
            //assert(m_current)
            if (m_current->m_children && !m_skipChildren)
            {
                // Visit the first child.
                m_current = m_current->m_children;
            }
            else
            {
                m_skipChildren = false;
                // This state has no child, so go to its sibling. If there is
                // no sibling, go to the parent's sibling, the parent's
                // parent's sibling... Note that the direct ancestors must
                // be skipped because they have been visited already.
                while (m_current->m_nextSibling == 0)
                {
                    m_current = m_current->parent();
                    if (!m_current)
                        return *this;
                }
                m_current = m_current->m_nextSibling;
            }
            return *this;
        }

        //! Postfix increment.
        PreOrderIterator operator++(int) noexcept
        {
            PreOrderIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(PreOrderIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(PreOrderIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

        //! \brief Skip child iteration.
        //!
        //! When this method is called, the iterator will skip the children
        //! when it is advanced the next time. The flag is reset automatically.
        void skipChildren() noexcept
        {
            m_skipChildren = true;
        }


        //! \brief Returns an iterator to the first child.
        //!
        //! Returns an iterator to the first child.
        sibling_iterator child_begin() noexcept
        {
            return sibling_iterator(m_current->m_children);
        }

        //! \brief Returns a const-iterator to the first child.
        //!
        //! Returns a const-iterator to the first child.
        const_sibling_iterator child_begin() const noexcept
        {
            return const_sibling_iterator(m_current->m_children);
        }

        //! \brief Returns a const-iterator to the first child.
        //!
        //! Returns a const-iterator to the first child.
        const_sibling_iterator child_cbegin() const noexcept
        {
            return const_sibling_iterator(m_current->m_children);
        }

        //! \brief Returns an iterator past the last child.
        //!
        //! Returns an iterator past the last child.
        sibling_iterator child_end() noexcept
        {
            return sibling_iterator();
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_end() const noexcept
        {
            return const_sibling_iterator();
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_cend() const noexcept
        {
            return const_sibling_iterator();
        }

    private:
        //! The current state.
        pointer m_current;
        //! The flag is set, when the children of a state have to be skipped.
        bool m_skipChildren;

        //! Constructs an iterator pointing to the \p state.
        PreOrderIterator(pointer state) noexcept
            : m_current(state),
              m_skipChildren(false)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend PreOrderIterator<true>;
    };

    //! \brief A view for pre-order iteration.
    template <bool TIsConst>
    class PreOrderView
    {
    public:
        using iterator = PreOrderIterator<TIsConst>;

        explicit PreOrderView(typename iterator::pointer state) noexcept
            : m_state(state)
        {
        }

        iterator begin() const noexcept
        {
            return m_state->pre_order_begin();
        }

        iterator end() const noexcept
        {
            return m_state->pre_order_end();
        }

    private:
        typename iterator::pointer m_state;
    };

    //! \brief A post-order iterator.
    //!
    //! The PostOrderIterator is a post-order depth-first iterator over a
    //! hierarchy of states. If \p TIsConst is set, it implements
    //! a const-iterator, otherwise the accessed state is mutable.
    template <bool TIsConst>
    class PostOrderIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef State value_type;
        typedef typename std::conditional<TIsConst, const State*, State*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const State&, State&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        PostOrderIterator() noexcept
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        PostOrderIterator(const PostOrderIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        PostOrderIterator& operator++() noexcept
        {
            //assert(m_current)

            if (m_current->m_nextSibling)
            {
                m_current = m_current->m_nextSibling;
                while (m_current->m_children)
                    m_current = m_current->m_children;
            }
            else
            {
                m_current = m_current->parent();
            }
            return *this;
        }

        //! Postfix increment.
        PostOrderIterator operator++(int) noexcept
        {
            PostOrderIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(PostOrderIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(PostOrderIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

    private:
        //! The current state.
        pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        PostOrderIterator(pointer state) noexcept
            : m_current(state)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend PostOrderIterator<true>;
    };

    //! \brief A view for post-order iteration.
    template <bool TIsConst>
    class PostOrderView
    {
    public:
        using iterator = PostOrderIterator<TIsConst>;

        explicit PostOrderView(typename iterator::pointer state) noexcept
            : m_state(state)
        {
        }

        iterator begin() const noexcept
        {
            return m_state->post_order_begin();
        }

        iterator end() const noexcept
        {
            return m_state->post_order_end();
        }

    private:
        typename iterator::pointer m_state;
    };

    //! \brief An iterator over state siblings.
    template <bool TIsConst>
    class SiblingIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef State value_type;
        typedef typename std::conditional<TIsConst, const State*, State*>::type
                    pointer;
        typedef typename std::conditional<TIsConst, const State&, State&>::type
                    reference;
        typedef std::forward_iterator_tag iterator_category;


        //! Default constructs an end-iterator.
        SiblingIterator() noexcept
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        SiblingIterator(const SiblingIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        SiblingIterator& operator++() noexcept
        {
            m_current = m_current->m_nextSibling;
            return *this;
        }

        //! Postfix increment.
        SiblingIterator operator++(int) noexcept
        {
            SiblingIterator temp(*this);
            ++*this;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(SiblingIterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(SiblingIterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the state.
        reference operator*() const noexcept
        {
            return *m_current;
        }

        //! Returns a pointer to the state.
        pointer operator->() const noexcept
        {
            return m_current;
        }

    private:
        //! The current child state.
        pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        SiblingIterator(pointer state) noexcept
            : m_current(state)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend SiblingIterator<true>;
    };

    // -------------------------------------------------------------------------
    // Transition iterators
    // -------------------------------------------------------------------------

    template <bool TIsConst>
    class TransitionIterator;

    //! The iterator type for iterating over transitions.
    typedef TransitionIterator<false> transition_iterator;
    //! The const-iterator type for iterating over transitions.
    typedef TransitionIterator<true> const_transition_iterator;


    transition_iterator beginTransitions()
    {
        return transition_iterator(m_transitions);
    }

    const_transition_iterator beginTransitions() const
    {
        return const_transition_iterator(m_transitions);
    }

    const_transition_iterator cbeginTransitions() const
    {
        return const_transition_iterator(m_transitions);
    }

    transition_iterator endTransitions()
    {
        return transition_iterator();
    }

    const_transition_iterator endTransitions() const
    {
        return const_transition_iterator();
    }

    const_transition_iterator cendTransitions() const
    {
        return const_transition_iterator();
    }


    //! \brief An iterator over a state's transitions.
    template <bool TIsConst>
    class TransitionIterator
    {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef transition_type value_type;
        typedef typename std::conditional<TIsConst, const transition_type*,
                                          transition_type*>::type pointer;
        typedef typename std::conditional<TIsConst, const transition_type&,
                                          transition_type&>::type reference;
        typedef std::forward_iterator_tag iterator_category;

        //! Default constructs an end-iterator.
        TransitionIterator()
            : m_current(0)
        {
        }

        //! A copy-constructor with implicit conversion from a non-const
        //! iterator.
        TransitionIterator(const TransitionIterator<false>& other)
            : m_current(other.m_current)
        {
        }

        //! Prefix increment.
        TransitionIterator& operator++()
        {
            m_current = m_current->m_nextInSourceState;
            return *this;
        }

        //! Postfix increment.
        TransitionIterator operator++ (int)
        {
            TransitionIterator temp(m_current);
            m_current = m_current->m_next;
            return temp;
        }

        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator== (TransitionIterator other) const
        {
            return m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!= (TransitionIterator other) const
        {
            return m_current != other.m_current;
        }

        //! Returns a reference to the transition.
        reference operator* () const
        {
            return *m_current;
        }

        //! Returns a pointer to the transition.
        pointer operator-> () const
        {
            return m_current;
        }

    private:
        //! The current transition.
        pointer m_current;

        //! Constructs an iterator pointing to the \p transition.
        TransitionIterator(pointer transition)
            : m_current(transition)
        {
        }

        friend class State;
        // Befriend the non-const version with the const version.
        friend TransitionIterator<true>;
    };

private:
    enum Flags
    {
        SkipTransitionSelection = 0x01,
        InEnterSet              = 0x02,
        InExitSet               = 0x04
    };

    //! The state's name.
    const char* m_name;
    //! The associated state machine.
    state_machine_type* m_stateMachine;
    //! The parent state.
    State* m_parent;
    //! A pointer to the first child.
    State* m_children;
    //! A pointer to the next sibling in the linked list.
    State* m_nextSibling;
    //! A pointer to the first transition.
    transition_type* m_transitions;
    //! This flag specifies if the children are active exclusively or parallely.
    ChildMode m_childMode;
    int m_flags; //! \todo This should be of type Flags

    bool m_internalActive;
    std::atomic_bool m_visibleActive;


    //! Adds a \p child.
    void addChild(State* child);
    //! Removes a \p child.
    void removeChild(State* child);

    //! Adds a \p transition.
    void pushBackTransition(transition_type* transition) noexcept;

    friend state_machine_type;

    template <typename TDerived>
    friend class fsm11_detail::EventDispatcherBase;
};

template <typename TOptions>
State<TOptions>::State(const char* name, State* parent)
    : m_name(name),
      m_stateMachine(parent ? parent->m_stateMachine : 0),
      m_parent(parent),
      m_children(0),
      m_nextSibling(0),
      m_transitions(0),
      m_childMode(Exclusive),
      m_internalActive(false),
      m_visibleActive(false)
{
    if (parent)
        parent->addChild(this);
}

template <typename TOptions>
State<TOptions>* State<TOptions>::findChild(StringRef name) const
{
#if 0
    const State* parent = this;
    while (1)
    {
        std::pair<StringRef, StringRef> splitName = name.split('.');
        if (splitName.first.empty())
            return 0;

        const_child_iterator child = parent->child_begin();
        for (; child != parent->child_end(); ++child)
        {
            if (child->name() == splitName.first)
                break;
        }
        if (child == parent->child_end())
            return 0;
        if (splitName.second.empty())
            return const_cast<State*>(&*child);

        parent = &*child;
        name = splitName.second;
    }
#endif
    return 0;
}

template <typename TOptions>
bool State<TOptions>::isActive() const
{
    return m_visibleActive;
}

template <typename TOptions>
void State<TOptions>::setParent(State* parent)
{
    if (parent == m_parent)
        return;

    if (m_parent)
        m_parent->removeChild(this);
    if (parent)
        parent->addChild(this);

    // Propagate the state machine of the new parent to the new child states.
    state_machine_type* fsm = parent ? parent->stateMachine() : 0;
    for (auto& state : *this)
        state.m_stateMachine = fsm;

    m_parent = parent;
}

// ----=====================================================================----
//     Private methods
// ----=====================================================================----

template <typename TOptions>
void State<TOptions>::addChild(State* child)
{
    assert(child->m_nextSibling == 0);

    if (!m_children)
    {
        m_children = child;
    }
    else
    {
        State* iter = m_children;
        while (iter->m_nextSibling)
            iter = iter->m_nextSibling;
        iter->m_nextSibling = child;
    }
}

template <typename TOptions>
void State<TOptions>::removeChild(State* child)
{
    assert(m_children != 0);

    if (child == m_children)
        m_children = child->m_nextSibling;
    else
    {
        State* iter = m_children;
        while (1)
        {
            assert(iter != 0);
            if (iter->m_nextSibling == child)
            {
                iter->m_nextSibling = child->m_nextSibling;
                break;
            }
            iter = iter->m_nextSibling;
        }
    }
    child->m_nextSibling = 0;
}

template <typename TOptions>
void State<TOptions>::pushBackTransition(transition_type* transition) noexcept
{
    if (!m_transitions)
    {
        m_transitions = transition;
    }
    else
    {
        transition_type* iter = m_transitions;
        while (iter->m_nextInSourceState)
            iter = iter->m_nextInSourceState;
        iter->m_nextInSourceState = transition;
    }
}

// ----=====================================================================----
//     Free functions
// ----=====================================================================----

//! Returns the least common proper ancestor.
//!
//! Returns the least common proper ancestor of the states \p state1 and
//! \p state2. A state \p S is the least common proper ancestor if it
//! is a proper ancestor of both \p state1 and \p state2 and no descendant
//! of \p S has this property.
template <typename TOptions>
State<TOptions>* findLeastCommonProperAncestor(
        State<TOptions>* state1, State<TOptions>* state2)
{
    while (state1)
    {
        if (isProperAncestor(state1, state2))
            return state1;
        state1 = state1->parent();
    }

    return 0;
}

//! \brief Checks if a state is an ancestor of another state.
//!
//! Returns \p true, if \p ancestor is an ancestor of \p descendant. Every
//! state is its own ancestor.
template <typename TOptions>
bool isAncestor(const State<TOptions>* ancestor,
                const State<TOptions>* descendant)
{
    while (descendant)
    {
        if (ancestor == descendant)
            return true;
        descendant = descendant->parent();
    }
    return false;
}

template <typename TOptions>
inline
bool isProperAncestor(const State<TOptions>* ancestor,
                      const State<TOptions>* descendant)
{
    return isAncestor(ancestor, descendant->parent());
}

template <typename TOptions>
inline
bool isDescendant(const State<TOptions>* descendant,
                  const State<TOptions>* ancestor)
{
    return isAncestor(ancestor, descendant);
}

} // namespace fsm11

#endif // FSM11_STATE_HPP
