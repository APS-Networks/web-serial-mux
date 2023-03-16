#pragma once


#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/trie_policy.hpp>
#include <ext/pb_ds/detail/trie_policy/trie_policy_base.hpp>
namespace pbds = __gnu_pbds;


namespace apsn::detail {

#define PB_DS_TRIE_POLICY_BASE \
  pbds::detail::trie_policy_base<Node_CItr,Node_Itr,_ATraits, _Alloc>

template<typename Node_CItr,
    typename Node_Itr,
    typename _ATraits,
    typename _Alloc>
struct lpm_node_update : private PB_DS_TRIE_POLICY_BASE
{
private:
    using base_type = PB_DS_TRIE_POLICY_BASE;
public:
    using key_const_reference = typename base_type::key_const_reference;
    using access_traits       = _ATraits;
    using node_iterator       = Node_Itr;
    using node_const_iterator = Node_CItr;
    using iterator            = typename node_iterator::value_type;
    using const_iterator      = typename node_const_iterator::value_type;
    using metadata_type       = pbds::null_type;
    using a_const_iterator    = typename access_traits::const_iterator;
    using allocator_type      = _Alloc;

    using size_type           = typename allocator_type::size_type;

    auto longest_match(key_const_reference key) -> const_iterator;
protected:
    auto operator()(node_iterator, node_const_iterator) const -> void
    {
        /* No node metadata to update*/
    }
private:
    auto next_child(node_iterator nd_it,
            typename access_traits::const_iterator b,
	        typename access_traits::const_iterator e, 
            node_iterator end_nd_it,
	        const access_traits& r_traits) -> node_iterator;

    /// Returns the const iterator associated with the just-after last element.
    virtual const_iterator
    end() const = 0;

    /// Returns the iterator associated with the just-after last element.
    virtual iterator
    end() = 0;

    /// Returns the node_const_iterator associated with the trie's root node.
    virtual node_const_iterator
    node_begin() const = 0;

    /// Returns the node_iterator associated with the trie's root node.
    virtual node_iterator
    node_begin() = 0;

    /// Returns the node_const_iterator associated with a just-after leaf node.
    virtual node_const_iterator
    node_end() const = 0;

    /// Returns the node_iterator associated with a just-after leaf node.
    virtual node_iterator
    node_end() = 0;

    /// Access to the cmp_fn object.
    virtual const access_traits&
    get_access_traits() const = 0;
};


#define LPM_NODE_CLASS lpm_node_update<Node_CItr, Node_Itr, _ATraits, _Alloc>


template<typename Node_CItr,
    typename Node_Itr,
    typename _ATraits,
    typename _Alloc>
auto LPM_NODE_CLASS::longest_match(key_const_reference key) -> const_iterator
{
    auto nodes_curr = node_begin(),
         nodes_last = node_end(), 
         nodes_end  = node_end();

    const auto& a_traits = get_access_traits();
    auto key_begin = std::begin(key), 
         key_end   = std::end(key);

    const size_type given_range_length = std::distance(key_begin, key_end);

    while (true) {
        if (nodes_curr == nodes_end) {
            return nodes_last != nodes_end ? *nodes_last : end();
        }

        auto common_range_length = base_type::common_prefix_len(
                nodes_curr,
                key_begin,
                key_end,
                a_traits);

        if (common_range_length > given_range_length) {
            return end();
        }
        nodes_last = nodes_curr;
        nodes_curr = next_child(nodes_curr,
                key_begin,
                key_end,
                nodes_end,
                a_traits);
    }
}

template<typename Node_CItr,
    typename Node_Itr,
    typename _ATraits,
    typename _Alloc>
auto LPM_NODE_CLASS::next_child(node_iterator nd_it,
        typename access_traits::const_iterator b,
        typename access_traits::const_iterator e, 
        node_iterator end_nd_it,
        const access_traits& r_traits) -> node_iterator
{
    auto const num_children = nd_it.num_children();
    auto ret = end_nd_it;

    size_type max_length = 0;
    for (size_type i = 0; i < num_children; ++i) {
        node_iterator pot = nd_it.get_child(i);
        const size_type common_range_length =
        base_type::common_prefix_len(pot, b, e, r_traits);

        if (common_range_length > max_length) {
            ret = pot;
            max_length = common_range_length;
        }
    }
    return (ret);
}



template <typename Key, typename Mapped>
using lpm_map = pbds::trie<
            Key,
            Mapped,
            pbds::trie_string_access_traits<Key>,
            pbds::pat_trie_tag,
            detail::lpm_node_update
            >;


/* Helper to allow types convertible to Key and Mapped. */
template <typename Key, typename Mapped,
        typename KeyIn, typename MappedIn>
auto insert(lpm_map<Key,Mapped> & obj, KeyIn && key, MappedIn && mapped)
{
    return obj.insert(std::pair<Key, Mapped>(key, mapped));
}


}