;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=456, seed=6

(define (problem blocksworld-456)
 (:domain blocksworld)
 (:objects b1 b2 b3 b4 - object)
 (:init 
    (clear b2)
    (clear b1)
    (on b1 b3)
    (on b2 b4)
    (on-table b3)
    (on-table b4))
 (:goal  (and 
    (clear b2)
    (clear b1)
    (on b2 b3)
    (on b1 b4)
    (on-table b3)
    (on-table b4))))
