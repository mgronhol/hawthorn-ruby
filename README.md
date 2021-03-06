hawthorn-ruby
=============
Hawthorn Graph Database as a gem


What is it?
=============

Hawthorn is a embeddable graph database written in C + Ruby. 

In graph databases you represent your data as nodes and connections ( edges ) between them. 
Each node can have multiple properties ( so they work a bit like a key-value store) and each edge has type and weight.

The point of using graph databases is two-fold: 

 * You can model your data more easily, most of the real world problems involve relations between objects. You can literally draw your database to paper.
 * You want to exploit the locality of your data, why should a query like "get all friends of user A" take more time when theres more non-related entries in the database?

Currently Hawthorn runs in non-persistent in-memory mode. This will change in future.

See examples & wikipedia for more info.

Installation
=============

``gem install hawthorn``

Examples
=============

## Hawthorn::Database

    db = Hawthorn::Database.new
    node = db.create_node( :property1 => "All properties", :property2 => "are strings" )
    
    cars = db.find( "type", "car" )
    
    db.connect( node1, node2, "edge_type" )
    db.connect( node1, node2, "edge_type2", 1.0 ) #added weight
    
    db.all_nodes.each do |node|
        puts node.props["name"]
    end
        

## Hawthorn::Node

    node.set( "property", "value" )
    
    puts node.props.to_s
    
	if node.is_connected?( node2, "read_access" )
       puts "Read access"
    end
    
    out_edges = node.outbound( "read_access" )
    in_edges = node.inbound( "belongs_to" )

## Hawthorn::Edge

    edges = node.outbound( "friends" )
    edges.each do |edge|
       puts edge.source.props["name"] + " -> " + edge.target.props["name"]
    end

## Hawthorn::Query

    # Get Alfonsos friends-of-friends
    foaf = db.query
            .start( "name", "Alfonso" )
            .out( "is_friend" )
            .out( "is_friend" )
            .results
   

