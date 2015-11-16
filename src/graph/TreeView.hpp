#pragma once

#include "TransformGraphTypes.hpp"
namespace envire { namespace core
{
    /** Structure to store the parent and children relation for a vertex in a tree.
     */
    struct VertexRelation
    {
        vertex_descriptor parent;
        std::unordered_set<vertex_descriptor> children;
    };

    /**A map that shows the vertex information (parent and children) of the vertices in a tree.
       The key is the vertex descriptor.*/
    using VertexRelationMap = std::unordered_map<vertex_descriptor, VertexRelation>;
    
    
    class TreeView;
    /**A class that notifies TreeViews about updates */
    struct TreeUpdatePublisher
    {
        virtual ~TreeUpdatePublisher() {}
        /**Unsubscribe the view from the publisher */
        virtual void unsubscribeTreeView(TreeView* view) = 0;
        /**Subscribe the view to the publisher */
        virtual void subscribeTreeView(TreeView* view) = 0;
    };
    
    /** A TreeView is a tree shaped snapshot of the graph structure.
     *  it is generated by traversing the graph in bfs order, starting from a 
     *  root node. The vertex_descriptors used in the graph are pointers to
     *  vertices in the graph and can be used to manipulate the graph.
     */
    class TreeView
    {
    public:
      
        TreeView() {}
        
        /**Creates a copy ***without*** retaining the treeUpdated subscribers  */
        TreeView(const TreeView& other)
        {
            *this = other;
        }
        
        /**Creates a copy ***without*** retaining the treeUpdated subscribers  */
        TreeView& operator=(const TreeView& other)
        {
            //this operator has to be here because the default
            //operator= wants to copy treeUpdated, which is is not possible.
            
            //WARNING If you add members to this class, make sure
            //        to copy them!
            publisher = other.publisher;
            tree = other.tree;
            crossEdges = other.crossEdges;
            return *this;
        }
        
        TreeView(TreeView&& other) noexcept : tree(std::move(other.tree)),
                                              crossEdges(std::move(other.crossEdges))                     
        {
            //if the other TreeView was subscribed, unsubscribe it and 
            //subscribe this instead
            if(other.publisher != nullptr)
            {
              other.publisher->unsubscribeTreeView(&other);
              other.publisher->subscribeTreeView(this); //sets this.publisher
              other.publisher = nullptr;
            }
            treeUpdated.swap(other.treeUpdated); //there is no move ctor, therfore swap
        }
        
        
        virtual ~TreeView()
        {
            if(nullptr != publisher)
            {
                publisher->unsubscribeTreeView(this);
                publisher = nullptr;
            }
        }
        
        /**If a publisher is set the TreeView will automatically unsubscribe 
        * from the publisher on destruction. */
        void setPublisher(TreeUpdatePublisher* pub)
        {
            assert(publisher == nullptr);
            publisher = pub;
        }

        bool isRoot(const vertex_descriptor vd) const
        {
            if (this->tree.at(vd).parent == LabeledTransformGraph::null_vertex())
                return true;
            else
                return false;
        }
        
        /**This signal is invoked whenever the tree is updated by the TransformGraph
        * @note This is only the case if you requested an updating TreeView. 
        *       Otherwise this signal will never be invoked.
        */
        boost::signals2::signal<void ()> treeUpdated;
        
        VertexRelationMap tree;

        /*The edges, that had to be removed to create the tree.
        *I.e. All edges that lead to a vertex that has already been discovered.
        *This does **not** include back-edges. I.e. edges that lead to a vertex that
        *has already been visited. */
        std::vector<edge_descriptor> crossEdges;
        
    protected:
        TreeUpdatePublisher* publisher = nullptr;/*< Used for automatic unsubscribing in dtor */
    };
}}