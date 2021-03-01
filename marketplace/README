Computer Systems Architecture -- Assignment 1

I chose to implement the solution with locks. Their usage is quite straightforward: every time a certain resource is needed, the current thread will lock it until its job is done.
In this case, 2 locks are needed: one for the actions of a producer, and another for a consumer.

Producer:
    - first and foremost the producer registers to the marketplace;
    - the producer will try to publish their products until there is nothing left, therefore "while 1" is needed;
    - the products will be published one by one until the quantity in the input is reached.

Consumer:
    - a consumer will go through all the operations that define the carts;
    - the type of the current operation will define the method to be called (add/remove from cart)
    - when all the operations in a cart are finished, the items will be ordered and printed.

Marketplace:
    I added the following:
        - producer_lock;
        - consumer_lock;
        - prod_queue: a list of producer lists;
        - cons_carts: a list of carts.

    Both producer lists and carts will contain elements in the form of dictionaries, containing the product and its producer, for easier data manipulation.
    To give an example, for 2 registered producers who published 2 products, prod_queue will look like this:
         [
            [{'product': product_1, 'producer': list_id_1}, {'product': product_2, 'producer': list_id_1}],
            [{'product': product_1, 'producer': list_id_2}, {'product': product_2, 'producer': list_id_2}]
         ]

    register_producer:
        - the current length of the prod_queue corresponds to the position of the producer list to be added;
        - before returning the value, an empty list is inserted, representing the new producer list.

    new_cart:
        - works on the same principles as register_producer.

    add_to_cart:
        - looping through all the products in all the producer lists until one that fits our needs is found;
        - the product is added to cart and removed from the producer list.

    remove_from_cart:
        - looping through the cart until the product to be removed is found;
        - the product is removed from the cart and added back to the producer list.

    place_order:
        - the cart contains dictionaries out of which only the 'product' fields are of interest;
        - the data in those fields are selected and put into a list which is returned.

