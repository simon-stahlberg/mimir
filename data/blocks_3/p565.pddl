;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=565, seed=25

(define (problem blocksworld-565)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on-table b2)
    (clear b3)
    (on b3 b5)
    (on-table b5)
    (clear b4)
    (on b4 b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1)
    (clear b2)
    (on b2 b4)
    (on-table b4)
    (clear b5)
    (on b5 b3)
    (on-table b3))))
