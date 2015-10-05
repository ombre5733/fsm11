/*******************************************************************************
  fsm11 - A C++11-compliant framework for finite state machines

  Copyright (c) 2015, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef FSM11_STATE_HPP
#define FSM11_STATE_HPP

#include "statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/atomic.hpp>
#include <weos/exception.hpp>
#include <weos/initializer_list.hpp>
#include <weos/type_traits.hpp>
#else
#include <atomic>
#include <exception>
#include <initializer_list>
#include <type_traits>
#endif // FSM11_USE_WEOS

#include <cstring>
#include <iterator>

namespace fsm11
{

//! \brief The possible child modes.
//!
//! This enum lists the possible child modes of a state.
//! - Exclusive: The children are active exclusively (i.e. one and only
//!   one child will be active).
//! - Parallel: The children are active parallely (i.e. all children
//!   will be active simultanously).
enum class ChildMode
{
    Exclusive,
    Parallel
};

//! \brief A state in a state machine.
//!
//! The State defines a state in a finite state machine.
template <typename TStateMachine>
class State
{
    using options = typename fsm11_detail::get_options<TStateMachine>::type;

public:
    using event_type = typename options::event_type;
    using state_machine_type = TStateMachine;
    using transition_type = Transition<TStateMachine>;
    using type = State<TStateMachine>;


    //! \brief Constructs a state.
    //!
    //! Constructs a state with given \p name which will be a child of the
    //! \p parent state. The \p parent may be a null-pointer. In this case
    //! the state is at the root of its hierarchy.
    explicit State(const char* name, State* parent = nullptr) noexcept;

    //! \brief Destroys the state.
    virtual ~State() {}

    State(const State&) = delete;
    State& operator=(const State&) = delete;

    //! \brief Returns the child mode.
    //!
    //! Returns the current child mode. The default is Exclusive.
    ChildMode childMode() const noexcept
    {
        return ChildMode(m_flags & ChildModeFlag);
    }

    //! Finds a child.
    //!
    //! Returns a pointer to the child with the given \p name or a
    //! null-pointer if no such child exists.
    State* findChild(const char* name) const noexcept;

    //! Finds a descendant.
    //!
    //! Recursively looks for a descendant state. On each hierarchy level the
    //! a child with a name from the corresponding \p nameList element is
    //! searched.
    //!
    //! For example, the expression
    //! \code
    //! s.findChild({"A", "B"});
    //! \endcode
    //! returns a pointer to the grand-child \p B, which is a child of
    //! state \p A, which in turn is a child of \p s.
    State* findDescendant(
            std::initializer_list<const char*> nameList) const noexcept;

    //! \brief Returns the initial state.
    //!
    //! Returns the initial state. By default, this is a null-pointer.
    //!
    //! \sa setInitialState()
    State* initialState() const noexcept
    {
        return m_initialState;
    }

    //! \brief Checks if the state is active.
    //!
    //! Returns \p true, if the state is active, which means that it belongs
    //! to the current state machine configuration.
    bool isActive() const noexcept;

    //! \brief Checks for atomicity.
    //!
    //! Returns \p true, if this state is atomic which means that it does not
    //! have children.
    bool isAtomic() const noexcept
    {
        return m_children == nullptr;
    }

    //! \brief Checks for a compound state.
    //!
    //! Returns \p true, if this state is a compound state, i.e. it does
    //! have at least one child and the children are active exclusively.
    //! One and only one child of an active compound state will be active.
    bool isCompound() const noexcept
    {
        return !isAtomic()
               && ChildMode(m_flags & ChildModeFlag) == ChildMode::Exclusive;
    }

    //! \brief Checks for a parallel state.
    //!
    //! Returns \p true, if this state is a parallel state, i.e. it does
    //! have at least one child and the children are active in parallel.
    //! All children of an active parallel state will be active.
    bool isParallel() const noexcept
    {
        return !isAtomic()
               && ChildMode(m_flags & ChildModeFlag) == ChildMode::Parallel;
    }

    //! \brief The name.
    //!
    //! Returns the state's name.
    const char* name() const noexcept
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
    State* parent() const noexcept
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
    void setChildMode(ChildMode mode) noexcept
    {
        m_flags &= ~ChildModeFlag;
        m_flags |= static_cast<int>(mode);
    }

    //! \brief Sets the initial state.
    //!
    //! Sets the initial state to \p descendant. If \p descendant is no
    //! proper descendant of this state, an exception is thrown.                TODO: which exception?
    //!
    //! The initial state will be entered, if this state or any of its
    //! ancestors is the target of a transition and no other transition
    //! targets a descendant.
    void setInitialState(State* descendant);

    //! \brief Changes the parent.
    //!
    //! Removes this child from its old parent and makes it a child of the
    //! new \p parent state. If \p parent is a null-pointer, this state will
    //! be at the root of its hieararchy.
    //!
    //! \note Changing the parent state while the associated state machine
    //! is running results in undefined behaviour.
    void setParent(State* parent) noexcept;

    //! \brief Returns the state machine.
    //!
    //! Returns the state machine to which this state belongs. If the state
    //! has not been added to a state machine, yet, a null-pointer is
    //! returned.
    state_machine_type* stateMachine() const noexcept
    {
        return m_stateMachine;
    }

    // -------------------------------------------------------------------------
    // State iterators
    // -------------------------------------------------------------------------

    template <typename TState>
    class PreOrderIterator;
    typedef PreOrderIterator<State> pre_order_iterator;
    typedef PreOrderIterator<const State>  const_pre_order_iterator;
    typedef pre_order_iterator iterator;
    typedef const_pre_order_iterator const_iterator;

    template <typename TState>
    class PreOrderSubtree;

    template <typename TState>
    class PostOrderIterator;
    typedef PostOrderIterator<State> post_order_iterator;
    typedef PostOrderIterator<const State>  const_post_order_iterator;

    template <typename TState>
    class PostOrderSubtree;

    template <typename TState>
    class SiblingIterator;
    typedef SiblingIterator<State> sibling_iterator;
    typedef SiblingIterator<const State> const_sibling_iterator;

    template <typename TState>
    class AtomicIterator;
    typedef AtomicIterator<State> atomic_iterator;
    typedef AtomicIterator<const State> const_atomic_iterator;


    //! \brief A pre-order iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_begin().
    pre_order_iterator begin() noexcept
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_begin().
    const_pre_order_iterator begin() const noexcept
    {
        return pre_order_begin();
    }

    //! \brief A pre-order const-iterator to the first state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator to the first state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_cbegin().
    const_pre_order_iterator cbegin() const noexcept
    {
        return pre_order_cbegin();
    }

    //! \brief A pre-order iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_end().
    pre_order_iterator end() noexcept
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_end().
    const_pre_order_iterator end() const noexcept
    {
        return pre_order_end();
    }

    //! \brief A pre-order const-iterator past the last state of the sub-tree.
    //!
    //! Returns a pre-order const-iterator past the last state of the sub-tree
    //! rooted at this state. This is a synonym for pre_order_cend().
    const_pre_order_iterator cend() const noexcept
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
    //! rooted at this state.
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
    PreOrderSubtree<State> pre_order_subtree() noexcept
    {
        return PreOrderSubtree<State>(this);
    }

    PreOrderSubtree<const State> pre_order_subtree() const noexcept
    {
        return PreOrderSubtree<const State>(this);
    }


    //! \brief A post-order iterator to the first state of the sub-tree.
    //!
    //! Returns a post-order iterator to the first state of the sub-tree
    //! rooted at this state.
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
    //! rooted at this state.
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

    PostOrderSubtree<State> post_order_subtree() noexcept
    {
        return PostOrderSubtree<State>(this);
    }

    PostOrderSubtree<const State> post_order_subtree() const noexcept
    {
        return PostOrderSubtree<const State>(this);
    }


    //! \brief An iterator to the first child.
    //!
    //! Returns an iterator to the first child.
    sibling_iterator child_begin() noexcept
    {
        return sibling_iterator(m_children);
    }

    //! \brief A const-iterator to the first child.
    //!
    //! Returns a const-iterator to the first child.
    const_sibling_iterator child_begin() const noexcept
    {
        return sibling_iterator(m_children);
    }

    //! \brief A const-iterator to the first child.
    //!
    //! Returns a const-iterator to the first child.
    const_sibling_iterator child_cbegin() const noexcept
    {
        return sibling_iterator(m_children);
    }

    //! \brief An iterator past the last child.
    //!
    //! Returns an iterator past the last child.
    sibling_iterator child_end() noexcept
    {
        return sibling_iterator(nullptr);
    }

    //! \brief A const-iterator past the last child.
    //!
    //! Returns a const-iterator past the last child.
    const_sibling_iterator child_end() const noexcept
    {
        return sibling_iterator(nullptr);
    }

    //! \brief A const-iterator past the last child.
    //!
    //! Returns a const-iterator past the last child.
    const_sibling_iterator child_cend() const noexcept
    {
        return sibling_iterator(nullptr);
    }


    //! \brief An iterator to the first atomic state of the sub-tree.
    //!
    //! Returns an iterator to the first atomic state of the sub-tree
    //! rooted at this state.
    atomic_iterator atomic_begin() noexcept
    {
        State* state = this;
        while (state->m_children)
            state = state->m_children;
        return atomic_iterator(state);
    }

    //! \brief A const-iterator to the first atomic state of the sub-tree.
    //!
    //! Returns a const-iterator to the first atomic state of the sub-tree
    //! rooted at this state.
    const_atomic_iterator atomic_begin() const noexcept
    {
        return atomic_cbegin();
    }

    //! \brief A const-iterator to the first atomic state of the sub-tree.
    //!
    //! Returns a const-iterator to the first atomic state of the sub-tree
    //! rooted at this state.
    const_atomic_iterator atomic_cbegin() const noexcept
    {
        return const_cast<State*>(this)->atomic_begin();
    }

    //! \brief An iterator past the last atomic state of the sub-tree.
    //!
    //! Returns an iterator past the last atomic state of the sub-tree
    //! rooted at this state.
    atomic_iterator atomic_end() noexcept
    {
        State* state = this;
        while (state && state->m_nextSibling == nullptr)
        {
            state = state->parent();
        }
        if (state)
        {
            state = state->m_nextSibling;
            while (state->m_children)
                state = state->m_children;
        }
        return atomic_iterator(state);
    }

    //! \brief A const-iterator past the last atomic state of the sub-tree.
    //!
    //! Returns a const-iterator past the last atomic state of the sub-tree
    //! rooted at this state.
    const_atomic_iterator atomic_end() const noexcept
    {
        return atomic_cend();
    }

    //! \brief A const-iterator past the last atomic state of the sub-tree.
    //!
    //! Returns a const-iterator past the last atomic state of the sub-tree
    //! rooted at this state.
    const_atomic_iterator atomic_cend() const noexcept
    {
        return const_cast<State*>(this)->atomic_end();
    }

private:
    template <typename T, typename TDerived>
    struct IteratorBase
    {
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = typename FSM11STD::add_pointer<T>::type;
        using reference = typename FSM11STD::add_lvalue_reference<T>::type;
        using iterator_category = std::forward_iterator_tag;


        //! Returns \p true, if this iterator is equal to the \p other iterator.
        bool operator==(TDerived other) const noexcept
        {
            return derived().m_current == other.m_current;
        }

        //! Returns \p true, if this iterator is not equal to the \p other
        //! iterator.
        bool operator!=(TDerived other) const noexcept
        {
            return derived().m_current != other.m_current;
        }

        //! Postfix increment.
        TDerived operator++(int) noexcept
        {
            TDerived temp(derived());
            ++derived();
            return temp;
        }

        //! Returns a pointer to the accessed element.
        pointer operator->() const noexcept
        {
            return derived().m_current;
        }

        //! Returns a reference to the accessed element.
        reference operator*() const noexcept
        {
            return *derived().m_current;
        }

    private:
        TDerived& derived()
        {
            return *static_cast<TDerived*>(this);
        }

        const TDerived& derived() const
        {
            return *static_cast<const TDerived*>(this);
        }
    };

public:
    //! \brief A pre-order iterator.
    //!
    //! The PreOrderIterator is a pre-order depth-first iterator over a
    //! hierarchy of states of type \p TState.
    template <typename TState>
    class PreOrderIterator : public IteratorBase<TState,
                                                 PreOrderIterator<TState>>
    {
        using base_type = IteratorBase<TState, PreOrderIterator<TState>>;

    public:
        PreOrderIterator() noexcept = default;
        PreOrderIterator(const PreOrderIterator&) noexcept = default;
        PreOrderIterator& operator=(const PreOrderIterator&) noexcept = default;

        //! Constructs a const-iterator from a non-const iterator.
        template <bool B = FSM11STD::is_const<TState>::value,
                  typename = typename FSM11STD::enable_if<B>::type>
        PreOrderIterator(
                const PreOrderIterator<
                typename FSM11STD::remove_cv<TState>::type>& other) noexcept
            : m_current(other.m_current),
              m_skipChildren(false)
        {
        }

        using base_type::operator++;

        //! Prefix increment.
        PreOrderIterator& operator++() noexcept
        {
            FSM11_ASSERT(m_current);

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
                while (m_current->m_nextSibling == nullptr)
                {
                    m_current = m_current->parent();
                    if (!m_current)
                        return *this;
                }
                m_current = m_current->m_nextSibling;
            }
            return *this;
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
            return sibling_iterator(nullptr);
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_end() const noexcept
        {
            return const_sibling_iterator(nullptr);
        }

        //! \brief Returns a const-iterator past the last child.
        //!
        //! Returns a const-iterator past the last child.
        const_sibling_iterator child_cend() const noexcept
        {
            return const_sibling_iterator(nullptr);
        }

    private:
        //! The current state.
        typename base_type::pointer m_current;
        //! The flag is set, when the children of a state have to be skipped.
        bool m_skipChildren;

        //! Constructs an iterator pointing to the \p state.
        PreOrderIterator(typename base_type::pointer state) noexcept
            : m_current(state),
              m_skipChildren(false)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend PreOrderIterator<typename FSM11STD::add_const<TState>::type>;
    };

    //! \brief A view for pre-order iteration.
    template <typename TState>
    class PreOrderSubtree
    {
    public:
        using iterator = PreOrderIterator<TState>;

        explicit PreOrderSubtree(typename iterator::pointer state) noexcept
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
    //! hierarchy of states of type \p TState.
    template <typename TState>
    class PostOrderIterator : public IteratorBase<TState,
                                                  PostOrderIterator<TState>>
    {
        using base_type = IteratorBase<TState, PostOrderIterator<TState>>;

    public:
        PostOrderIterator() noexcept = default;
        PostOrderIterator(const PostOrderIterator&) noexcept = default;
        PostOrderIterator& operator=(const PostOrderIterator&) noexcept = default;

        //! Constructs a const-iterator from a non-const iterator.
        template <bool B = FSM11STD::is_const<TState>::value,
                  typename = typename FSM11STD::enable_if<B>::type>
        PostOrderIterator(
                const PostOrderIterator<
                typename FSM11STD::remove_cv<TState>::type>& other) noexcept
            : m_current(other.m_current)
        {
        }

        using base_type::operator++;

        //! Prefix increment.
        PostOrderIterator& operator++() noexcept
        {
            FSM11_ASSERT(m_current);

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

    private:
        //! The current state.
        typename base_type::pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        PostOrderIterator(typename base_type::pointer state) noexcept
            : m_current(state)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend PostOrderIterator<typename FSM11STD::add_const<TState>::type>;
    };

    //! \brief A view for post-order iteration.
    template <typename TState>
    class PostOrderSubtree
    {
    public:
        using iterator = PostOrderIterator<TState>;

        explicit PostOrderSubtree(typename iterator::pointer state) noexcept
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
    template <typename TState>
    class SiblingIterator : public IteratorBase<TState,
                                                SiblingIterator<TState>>
    {
        using base_type = IteratorBase<TState, SiblingIterator<TState>>;

    public:
        SiblingIterator() noexcept = default;
        SiblingIterator(const SiblingIterator&) noexcept = default;
        SiblingIterator& operator=(const SiblingIterator&) noexcept = default;

        //! Constructs a const-iterator from a non-const one.
        template <bool B = FSM11STD::is_const<TState>::value,
                  typename = typename FSM11STD::enable_if<B>::type>
        SiblingIterator(
                const SiblingIterator<
                typename FSM11STD::remove_cv<TState>::type>& other) noexcept
            : m_current(other.m_current)
        {
        }

        using base_type::operator++;

        //! Prefix increment.
        SiblingIterator& operator++() noexcept
        {
            m_current = m_current->m_nextSibling;
            return *this;
        }

    private:
        //! The current child state.
        typename base_type::pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        SiblingIterator(typename base_type::pointer state) noexcept
            : m_current(state)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend SiblingIterator<typename FSM11STD::add_const<TState>::type>;
    };

    //! \brief An iterator over atomic states.
    //!
    //! The AtomicIterator is an iterator the atomic states of type \p TState.
    template <typename TState>
    class AtomicIterator : public IteratorBase<TState,
                                               AtomicIterator<TState>>
    {
        using base_type = IteratorBase<TState, AtomicIterator<TState>>;
    public:
        AtomicIterator() noexcept = default;
        AtomicIterator(const AtomicIterator& other) noexcept = default;
        AtomicIterator& operator=(const AtomicIterator&) noexcept = default;

        //! Constructs a const-iterator from a non-const iterator.
        template <bool B = FSM11STD::is_const<TState>::value,
                  typename = typename FSM11STD::enable_if<B>::type>
        AtomicIterator(
                const AtomicIterator<
                typename FSM11STD::remove_cv<TState>::type>& other) noexcept
            : m_current(other.m_current)
        {
        }

        using base_type::operator++;

        //! Prefix increment.
        AtomicIterator& operator++() noexcept
        {
            FSM11_ASSERT(m_current);

            while (m_current->m_nextSibling == nullptr)
            {
                m_current = m_current->parent();
                if (!m_current)
                    return *this;
            }
            m_current = m_current->m_nextSibling;
            while (m_current->m_children)
                m_current = m_current->m_children;

            return *this;
        }

    private:
        //! The current state.
        typename base_type::pointer m_current;

        //! Constructs an iterator pointing to the \p state.
        AtomicIterator(typename base_type::pointer state) noexcept
            : m_current(state)
        {
        }


        friend class State;
        // Befriend the non-const version with the const version.
        friend AtomicIterator<typename FSM11STD::add_const<TState>::type>;
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


    transition_iterator beginTransitions() noexcept
    {
        return transition_iterator(m_transitions);
    }

    const_transition_iterator beginTransitions() const noexcept
    {
        return const_transition_iterator(m_transitions);
    }

    const_transition_iterator cbeginTransitions() const noexcept
    {
        return const_transition_iterator(m_transitions);
    }

    transition_iterator endTransitions() noexcept
    {
        return transition_iterator(nullptr);
    }

    const_transition_iterator endTransitions() const noexcept
    {
        return const_transition_iterator(nullptr);
    }

    const_transition_iterator cendTransitions() const noexcept
    {
        return const_transition_iterator(nullptr);
    }


    //! \brief An iterator over a state's transitions.
    template <bool TIsConst>
    class TransitionIterator : public IteratorBase<
                                        typename FSM11STD::conditional<TIsConst,
                                        const transition_type,
                                        transition_type>::type,
                                        TransitionIterator<TIsConst>>
    {
        using base_type = IteratorBase<typename FSM11STD::conditional<TIsConst,
                                       const transition_type,
                                       transition_type>::type,
                                       TransitionIterator<TIsConst>>;

    public:
        TransitionIterator() noexcept = delete;
        TransitionIterator(const TransitionIterator&) noexcept = default;
        TransitionIterator& operator=(const TransitionIterator&) noexcept = default;

        //! Constructs a const-iterator from a non-const one.
        template <bool B = TIsConst,
                  typename = typename FSM11STD::enable_if<B>::type>
        TransitionIterator(const TransitionIterator<false>& other) noexcept
            : m_current(other.m_current)
        {
        }

        using base_type::operator++;

        //! Prefix increment.
        TransitionIterator& operator++() noexcept
        {
            m_current = m_current->m_nextInSourceState;
            return *this;
        }

    private:
        //! The current transition.
        typename base_type::pointer m_current;

        //! Constructs an iterator pointing to the \p transition.
        TransitionIterator(typename base_type::pointer transition)
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
        SkipTransitionSelection = 0x100,
        InEnterSet              = 0x200,
        InExitSet               = 0x400,
        PartOfConflict          = 0x800,
        Transient               = 0xF00,

        ChildModeFlag           = 0x001,
        ShallowHistory          = 0x002,
        DeepHistory             = 0x004,
        StartInvoke             = 0x010,
        Active                  = 0x020,
        Invoked                 = 0x040,
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
    //! The initial state will be entered if this state is the target of
    //! a transition.
    State* m_initialState;
    //! A pointer to the first transition.
    transition_type* m_transitions;
    //! The flags.
    //! \todo This should be of type Flags
    int m_flags;

    FSM11STD::atomic_bool m_visibleActive;

    //! Adds a \p child.
    void addChild(State* child) noexcept;
    //! Removes a \p child.
    void removeChild(State* child) noexcept;

    //! Adds a \p transition.
    void pushBackTransition(transition_type* transition) noexcept;
    //! Deletes all transitions.
    template <typename TAlloc>
    void deleteTransitions(TAlloc& alloc) noexcept;

    friend state_machine_type;

    template <typename TDerived>
    friend class fsm11_detail::EventDispatcherBase;

    template <typename T>
    friend class HistoryState;
};

template <typename TStateMachine>
State<TStateMachine>::State(const char* name, State* parent) noexcept
    : m_name(name),
      m_stateMachine(parent ? parent->m_stateMachine : nullptr),
      m_parent(parent),
      m_children(nullptr),
      m_nextSibling(nullptr),
      m_initialState(nullptr),
      m_transitions(nullptr),
      m_flags(0),
      m_visibleActive(false)
{
    if (parent)
        parent->addChild(this);
}

template <typename TStateMachine>
State<TStateMachine>* State<TStateMachine>::findChild(
        const char* name) const noexcept
{
    for (const_sibling_iterator child = child_begin();
         child != child_end(); ++child)
    {
        if (std::strcmp(child->name(), name) == 0)
            return const_cast<State*>(&*child);
    }
    return nullptr;
}

template <typename TStateMachine>
State<TStateMachine>* State<TStateMachine>::findDescendant(
        std::initializer_list<const char*> nameList) const noexcept
{
    const State* state = this;
    for (auto&& name : nameList)
    {
        const_sibling_iterator child = state->child_begin();
        for (; child != state->child_end(); ++child)
        {
            if (std::strcmp(child->name(), name) == 0)
                break;
        }
        if (child == state->child_end())
            return nullptr;
        state = &*child;
    }
    return const_cast<State*>(state);
}

template <typename TStateMachine>
inline
bool State<TStateMachine>::isActive() const noexcept
{
    return m_visibleActive;
}

template <typename TStateMachine>
void State<TStateMachine>::setInitialState(State* descendant)
{
    //if (!isProperAncestor(this, descendant))
    //    throw ;

    m_initialState = descendant;
}

template <typename TStateMachine>
void State<TStateMachine>::setParent(State* parent) noexcept
{
    if (parent == m_parent)
        return;

    if (m_parent)
        m_parent->removeChild(this);
    if (parent)
        parent->addChild(this);

    // Propagate the state machine of the new parent to the new child states.
    state_machine_type* fsm = parent ? parent->stateMachine() : nullptr;
    for (auto& state : *this)
        state.m_stateMachine = fsm;

    m_parent = parent;
}

// ----=====================================================================----
//     Private methods
// ----=====================================================================----

template <typename TStateMachine>
void State<TStateMachine>::addChild(State* child) noexcept
{
    FSM11_ASSERT(child->m_nextSibling == nullptr);

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

template <typename TStateMachine>
void State<TStateMachine>::removeChild(State* child) noexcept
{
    FSM11_ASSERT(m_children != nullptr);

    if (child == m_children)
        m_children = child->m_nextSibling;
    else
    {
        State* iter = m_children;
        while (1)
        {
            FSM11_ASSERT(iter != nullptr);

            if (iter->m_nextSibling == child)
            {
                iter->m_nextSibling = child->m_nextSibling;
                break;
            }
            iter = iter->m_nextSibling;
        }
    }
    child->m_nextSibling = nullptr;
}

template <typename TStateMachine>
void State<TStateMachine>::pushBackTransition(
        transition_type* transition) noexcept
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

template <typename TStateMachine>
template <typename TAlloc>
void State<TStateMachine>::deleteTransitions(TAlloc& alloc) noexcept
{
    while (m_transitions)
    {
        auto next = m_transitions->m_nextInSourceState;
        m_transitions->~transition_type();
        alloc.deallocate(m_transitions, 1);
        m_transitions = next;
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
template <typename TStateMachine>
State<TStateMachine>* findLeastCommonProperAncestor(
        State<TStateMachine>* state1, State<TStateMachine>* state2) noexcept
{
    while ((state1 = state1->parent()) != nullptr)
    {
        if (isProperAncestor(state1, state2))
            return state1;
    }

    return nullptr;
}

//! \brief Checks if a state is an ancestor of another state.
//!
//! Returns \p true, if \p ancestor is an ancestor of \p descendant. Every
//! state is its own ancestor.
template <typename TStateMachine>
bool isAncestor(const State<TStateMachine>* ancestor,
                const State<TStateMachine>* descendant) noexcept
{
    if (!ancestor->isAtomic())
    {
        while (descendant)
        {
            if (ancestor == descendant)
                return true;
            descendant = descendant->parent();
        }
    }
    return false;
}

//! \brief Checks if a state is a proper ancestor of another state.
//!
//! Returns \p true, if \p ancestor is an ancestor of \p descendant
//! and \p ancestor and \p descendant are not the same.
//! In contrast to isAncestor(), isProperAncestor(s, s) is always false.
template <typename TStateMachine>
bool isProperAncestor(const State<TStateMachine>* ancestor,
                      const State<TStateMachine>* descendant) noexcept
{
    if (!ancestor->isAtomic())
    {
        while ((descendant = descendant->parent()) != nullptr)
        {
            if (ancestor == descendant)
                return true;
        }
    }
    return false;
}

template <typename TStateMachine>
inline
bool isDescendant(const State<TStateMachine>* descendant,
                  const State<TStateMachine>* ancestor) noexcept
{
    return isAncestor(ancestor, descendant);
}

} // namespace fsm11

#endif // FSM11_STATE_HPP
