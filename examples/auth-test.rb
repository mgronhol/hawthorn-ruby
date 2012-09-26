#!/usr/bin/env ruby

require 'hawthorn'

db = Hawthorn::Database.new

groups = {
	
	admins: db.create_node( :name => "admins", :type => "group" ),
	visitors: db.create_node( :name => "visitors", :type => "group" ),
	
}


users = {
	
	mauno: db.create_node( :name => "Mauno Mestaaja", :handle => "mauno", :type => "user" ),
	otto: db.create_node( :name => "Otto Olmi", :handle => "otto", :type => "user" )

}


machines = {
	novosibirsk: db.create_node( :name => "Novosibirsk", :type => "computer", :ip => "192.168.0.1" ),
	krasnojarsk: db.create_node( :name => "Krasnojarsk", :type => "computer", :ip => "192.168.0.2" )
}

db.connect( users[:otto], groups[:admins], "belongs_to" )
db.connect( users[:mauno], groups[:visitors], "belongs_to" )

db.connect( groups[:admins], machines[:novosibirsk], "read" )
db.connect( groups[:admins], machines[:novosibirsk], "write" )

db.connect( groups[:admins], machines[:krasnojarsk], "read" )
db.connect( groups[:admins], machines[:krasnojarsk], "write" )


db.connect( groups[:visitors], machines[:novosibirsk], "read" )


def has_read_access?(db, user_handle, computer_ip )
	computer = db.find( "ip", computer_ip )
	user = db.find( "handle", user_handle )
	
	if computer.size < 1 or user.size < 1
			return false
	end
	
	db.query
		.start(user[0])
		.crawl( ["belongs_to", "read"] )
		.filter { |props| props["type"] == "computer"  }
		.contains?( computer[0] )
end


puts "Otto:  can read 192.168.0.1?: " + has_read_access?(db, "otto", "192.168.0.1" ).to_s
puts "Mauno: can read 192.168.0.1?: " + has_read_access?(db, "mauno", "192.168.0.1" ).to_s
puts ""
puts "Otto:  can read 192.168.0.2?: " + has_read_access?(db, "otto", "192.168.0.2" ).to_s
puts "Mauno: can read 192.168.0.2?: " + has_read_access?(db, "mauno", "192.168.0.2" ).to_s



