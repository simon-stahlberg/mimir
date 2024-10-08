(define (problem roverprob2) (:domain rover)
(:requirements :strips :typing)
(:objects
	general - lander
	colour high_res - mode
	rover0 - rover
	rover0store - store
	waypoint0 waypoint1 - waypoint
	camera0 - camera
	objective0 - objective
	)
(:init
	(visible waypoint1 waypoint0)
	(visible waypoint0 waypoint1)
	(at_rock_sample waypoint0)
	(at_rock_sample waypoint1)
	(at_lander general waypoint1)
	(channel_free general)
	(at rover0 waypoint1)
	(available rover0)
	(store_of rover0store rover0)
	(empty rover0store)
	(equipped_for_soil_analysis rover0)
	(equipped_for_imaging rover0)
	(can_traverse rover0 waypoint1 waypoint0)
	(can_traverse rover0 waypoint0 waypoint1)
	(on_board camera0 rover0)
	(calibration_target camera0 objective0)
	(supports camera0 colour)
	(supports camera0 high_res)
	(visible_from objective0 waypoint1)
)

(:goal (and
(communicated_image_data objective0 high_res)
	)
)
)
