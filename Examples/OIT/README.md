Order-Independent Transparency (OIT)

Technique used to correctly render transparent objects without sorting them by depth.
Normally, transparency requires rendering objects from back to front, but OIT eliminates the need
of this sorting step.
This example uses the Per-Pixel Linked List technique, where a linked list buffer is used to store
all the fragments per pixel, and at the end sorts and blends them per pixel.