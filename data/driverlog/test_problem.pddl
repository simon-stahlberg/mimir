(define (problem DLOG-1-1-1)
	(:domain driverlog)
	(:requirements :typing)
	(:objects
	driver1 - driver
	truck1 - truck
	package1 - obj
	s0 - location
	s1 - location
	p0-1 - location
	)
	(:init
	(at driver1 s0)
	(at truck1 s0)
	(empty truck1)
	(at package1 s1)
	(path s0 p0-1)
	(path p0-1 s0)
	(path s1 p0-1)
	(path p0-1 s1)
	(link s0 s1)
	(link s1 s0)
)
	(:goal (and
	(at driver1 s0)
	(at truck1 s1)
	(at package1 s0)
	))


)
