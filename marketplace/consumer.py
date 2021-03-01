"""
This module represents the Consumer.

Computer Systems Architecture Course
Assignment 1
March 2020
"""
from threading import Thread
from time import sleep


class Consumer(Thread):
    """
    Class that represents a consumer.
    """

    def __init__(self, carts, marketplace, retry_wait_time, **kwargs):
        """
        Constructor.

        @type carts: List
        @param carts: a list of add and remove operations

        @type marketplace: Marketplace
        @param marketplace: a reference to the marketplace

        @type retry_wait_time: Time
        @param retry_wait_time: the number of seconds that a producer must wait
        until the Marketplace becomes available

        @type kwargs:
        @param kwargs: other arguments that are passed to the Thread's __init__()
        """
        Thread.__init__(self)
        self.carts = carts
        self.marketplace = marketplace
        self.retry_wait_time = retry_wait_time
        self.name = kwargs['name']

    def run(self):
        for cart_list in self.carts:
            # getting a cart from the marketplace
            cart = self.marketplace.new_cart()
            # looping through each operation in the list
            for operation in cart_list:
                for _ in range(operation['quantity']):
                    # checking the operation type
                    if operation['type'] == 'add':
                        # retry until successful in adding the product to cart
                        while not self.marketplace.add_to_cart(cart, operation['product']):
                            sleep(self.retry_wait_time)
                    else:
                        # the removal will be successful anyway if the product is in the cart
                        # if there is no such product in the cart, nothing will happen
                        self.marketplace.remove_from_cart(cart, operation['product'])
            # printing the items that were bought
            for product in self.marketplace.place_order(cart):
                print(self.name + " bought " + str(product))
