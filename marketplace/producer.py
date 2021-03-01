"""
This module represents the Producer.

Computer Systems Architecture Course
Assignment 1
March 2020
"""

from threading import Thread
from time import sleep


class Producer(Thread):
    """
    Class that represents a producer.
    """

    def __init__(self, products, marketplace, republish_wait_time, **kwargs):
        """
        @type products: List()
        @param products: a list of products that the producer will produce

        @type marketplace: Marketplace
        @param marketplace: a reference to the marketplace

        @type republish_wait_time: Time
        @param republish_wait_time: the number of seconds that a producer must
        wait until the marketplace becomes available

        @type kwargs:
        @param kwargs: other arguments that are passed to the Thread's __init__()
        """
        Thread.__init__(self)
        Thread.daemon = True
        self.products = products
        self.marketplace = marketplace
        self.republish_wait_time = republish_wait_time
        self.name = kwargs['name']

    def run(self):
        # registering the producer to the marketplace
        register_id = self.marketplace.register_producer()
        # the producer will try to place ALL their products in the marketplace
        while 1:
            for product in self.products:
                for _ in range(product[1]):
                    # retry until successful in publishing the product
                    while not self.marketplace.publish(register_id, product[0]):
                        sleep(self.republish_wait_time)
                    # waiting after publishing
                    sleep(product[2])
