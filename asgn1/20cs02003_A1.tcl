#Create a new simulator obj
set ns [new Simulator]

#Open the nam file asgn1.nam and variable trace file asgn1.tr
set namfile [open asgn1.nam w]
$ns namtrace-all $namfile

set tracefile [open asgn1.tr w]
$ns trace-all $tracefile

#A finish procedure
proc finish {} {
    global ns namfile tracefile
    $ns flush-trace
    close $namfile
    close $tracefile
    
    exec nam asgn1.nam &
    exit 0
}

#Create four nodes
set A [$ns node]
set B [$ns node]
set C [$ns node]
set D [$ns node]

$A color Blue
$A label A
$A shape box

$B color Red
$B label B

$D color Green
$D label D

$C color Blue
$C label C
$C shape box

$ns duplex-link $A $D 2Mb 40ms DropTail
$ns duplex-link $D $B 2Mb 40ms DropTail
$ns simplex-link $C $D 2Mb 40ms DropTail

$ns color 0 Red
$ns color 1 Blue
$ns color 2 Green

$ns duplex-link-op $A $D orient right-down
$ns simplex-link-op $C $D orient left-down
$ns duplex-link-op $D $B orient up

$ns queue-limit $D $B 10

$ns duplex-link-op $D $B queuePos 0.5

#---------------------------------------
# Setup a TCP connection for A
set tcp1 [new Agent/TCP]
$tcp1 set class_ 1
$ns attach-agent $A $tcp1

# Setup Sink for B 
set sink1 [new Agent/TCPSink]
$ns attach-agent $B $sink1
$ns connect $tcp1 $sink1
$tcp1 set fid_ 0

# Setup a FTP over TCP connection
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1
$ftp1 set type_ FTP

#----------------------------------------
# Setup a TCP connection for D
set tcp2 [new Agent/TCP]
$tcp2 set class_ 2
$ns attach-agent $D $tcp2

# Setup Sink for B
set sink2 [new Agent/TCPSink]
$ns attach-agent $B $sink2
$ns connect $tcp2 $sink2
$tcp2 set fid_ 1

# Setup a FTP over TCP connection
set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2
$ftp2 set type_ FTP

#----------------------------------------
# Setup a UDP connection
set udp1 [new Agent/UDP]
$ns attach-agent $C $udp1

set null [new Agent/Null]
$ns attach-agent $D $null
$ns connect $udp1 $null
$udp1 set fid_ 2

# Setup a CBR over UDP connection
set cbr1 [new Application/Traffic/CBR]
$cbr1 attach-agent $udp1
$cbr1 set type_ CBR
$cbr1 set packet_size_ 1000
$cbr1 set rate_ 1mb
$cbr1 set random_ false

$ns at 0.0 "$cbr1 start"
$ns at 0.0 "$ftp1 start"
$ns at 0.0 "$ftp2 start"
$ns at 1.0 "$ftp1 stop"
$ns at 1.0 "$ftp2 stop"
$ns at 1.0 "$cbr1 stop"
$ns at 1.1 "finish"

$ns run
