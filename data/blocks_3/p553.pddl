;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=553, seed=13

(define (problem blocksworld-553)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b3)
    (on b3 b1)
    (on-table b1)
    (clear b2)
    (on b2 b4)
    (on b4 b5)
    (on-table b5))
 (:goal  (and
    (clear b4)
    (on b4 b5)
    (on b5 b3)
    (on b3 b1)
    (on-table b1)
    (clear b2)
    (on-table b2))))
