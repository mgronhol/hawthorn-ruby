require 'hawthorn/hawthorn'

require 'Set'

module Hawthorn

	class Edge
		attr_reader :source, :target, :type, :weight
		def initialize( source, target, type, weight )
			@source = source
			@target = target
			@type = type
			@weight = weight
		end
	end

	class Node
		attr_reader :id, :props

		def initialize( id, props, db )
			@id = id
			@props = props
			@db = db
		end

		def set( key, value )
			@props[ key.to_s ] = value.to_s
			@db._set_property( @id, key.to_s, value.to_s )
		end

		def is_connected?( other, type )
			@db.are_connected?( self, other, type )
		end

		def outbound( type )
			@db.get_outbound_edges( self, type.to_s )
		end
		
		def inbound( type )
			@db.get_inbound_edges( self, type.to_s )
		end	
	end


	class Query
		attr_reader :results

		def initialize( db )
			@db = db
			@results = []
		end

		def start( *args )
			if args.size == 2
				@results = @db.find( args[0], args[1] )
			
			elsif args.size == 1
				@results = [args[0]]
			end

			self
		end
	
		def out( type )
			next_results = []
			visited = Set.new
			@results.each do |node|
				edges = node.outbound( type )
				edges.each do |edge|
					if not visited.include?( edge.target.id )
						visited.add( edge.target.id )
						next_results.push edge.target
					end
				end
			end
			@results = next_results

			self
		end

		def in( type )
			next_results = []
			visited = Set.new
			@results.each do |node|
				edges = node.outbound( type )
				edges.each do |edge|
					if not visited.include?( edge.source.id )
						visited.add( edge.source.id )
						next_results.push edge.source
					end
				end
			end
			@results = next_results

			self
		end
	
		def filter( &block )
			next_results = []
			@results.each do |node|
				next_results.push node if block.call( node.props )
			end
			
			@results = next_results

			self
		end

		def crawl( types )
			next_results = []
			visited = Set.new
			frontier = @results
			done = false
			while not done
				next_frontier = []
				types.each do |type|
					frontier.each do |node|
						conns = @db.get_connected( node, type.to_s )
						conns.each do |conn|
							if not visited.include?( conn.id )
								visited.add( conn.id )
								next_frontier.push conn
								next_results.push conn
							end
						end
					end
				end
				if next_frontier.size < 1
					done = true
				else
					frontier = next_frontier
				end
			end
			@results = next_results

			self
		end
	
		def contains?( target )
			@results.each do |node|
				if target.id == node.id
					return true
				end
			end
			false
		end
	end

	class Database

		def initialize
			@db = HawthornDB.new( 0 )
			@edge_types = {}
			@edge_types_invert = {}
			@edge_type_cnt = 1
		end

		def create_node( params = {} )
			node_id = @db.create_node
			props = @db.get_properties( node_id )
			
			node = self.get_node( node_id )
			
			params.each do |key, value|
				node.set( key.to_s, value.to_s )
			end	
			
			node
		end

		def _set_property( node_id, key, value )
			@db.set_property( node_id, key, value )
		end

		def get_node( node_id )
			props = @db.get_properties( node_id )
			
			Node.new( node_id, props, self )
		end

		def all_nodes
			node_ids = @db.all_nodes
			nodes = []
			node_ids.each do |node_id|
				nodes.push self.get_node( node_id )
			end
			
			nodes
		end

		def connect( source, target, type, weight = 1.0 )
			if not @edge_types.has_key?( type )
				@edge_types[type] = @edge_type_cnt
				@edge_types_invert[@edge_type_cnt] = type
				@edge_type_cnt += 1
			end
			type_id = @edge_types[type]
			
			@db.connect( source.id, target.id, type_id, weight )

		end

		def disconnect( source, target, type )
			if not @edge_types.has_key?( type )
				raise "Unknown edge type"
			end
			
			type_id = @edge_types[type]
			
			@db.disconnect( source.id, target.id, type_id )

		end

		def find( key, value )
			node_ids = @db.find( key.to_s, value.to_s )
			out = []
			node_ids.each do |node_id|
				out.push self.get_node( node_id )
			end
			
			out
		end

		def are_connected?( source, target, type )
			if not @edge_types.has_key?( type )
				raise "Unknown edge type"
			end
			
			type_id = @edge_types[type]

			@db.are_connected( source.id, target.id, type_id )
		end

		def get_connected( source, type )
			if not @edge_types.has_key?( type )
				raise "Unknown edge type"
			end
			
			type_id = @edge_types[type]
		
			node_ids = @db.get_connected( source.id, type_id )
			out = []

			node_ids.each do |node_id|
				out.push self.get_node( node_id )
			end
			out
		end

		def get_outbound_edges( source, type )
			if not @edge_types.has_key?( type )
				raise "Unknown edge type"
			end
			
			type_id = @edge_types[type]
			edges = @db.get_outbound( source.id, type_id )
			out = []
			edges.each do |edge|
				out.push Edge.new( self.get_node( edge[0] ), self.get_node( edge[1] ), @edge_types_invert[edge[2]], edge[3] ) 
			end
			out
		end

		def get_inbound_edges( source, type )
			if not @edge_types.has_key?( type )
				raise "Unknown edge type"
			end
			
			type_id = @edge_types[type]
			edges = @db.get_inbound( source.id, type_id )
			out = []
			edges.each do |edge|
				out.push Edge.new( self.get_node( edge[0] ), self.get_node( edge[1] ), @edge_types_invert[edge[2]], edge[3] ) 
			end
			out
		end

		def query
			Query.new( self )
		end
	end
end
