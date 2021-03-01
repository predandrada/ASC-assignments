"""
This module represents the Marketplace.

Computer Systems Architecture Course
Assignment 1
March 2020
"""
from threading import Lock


class Marketplace:
    """
    Class that represents the Marketplace. It's the central part of the implementation.
    The producers and consumers use its methods concurrently.
    """

    def __init__(self, queue_size_per_producer):
        """
        @type queue_size_per_producer: Int
        @param queue_size_per_producer: the maximum size of a queue associated with each producer
        """
        self.queue_size_per_producer = queue_size_per_producer
        self.prod_queue = []
        self.cons_carts = []
        self.producer_lock = Lock()
        self.consumer_lock = Lock()

    def register_producer(self):
        """
        @return: the register id of a producer
        """
        with self.producer_lock:
            register_id = len(self.prod_queue)
            self.prod_queue.append([])
        return register_id

    def publish(self, producer_id, product):
        """
        Tries to publish a product to the prod_queue according to the producer_id.
        @returns True or False. If the caller receives False, it should wait and then try again

        @type producer_id: Int
        @param producer_id: the id of the producer who initiated the publishing

        @type product: Product
        @param product: the product to be published
        """
        with self.producer_lock:
            # getting the size of the producer list
            current_size = len(self.prod_queue[producer_id])
            # checks whether there is space for another product
            if self.queue_size_per_producer > current_size:
                to_add = {
                    'product': product,
                    'producer': producer_id
                }
                self.prod_queue[producer_id].append(to_add)
                return True
        return False

    def new_cart(self):
        """
        @returns the id of a new cart
        """
        with self.consumer_lock:
            cart_id = len(self.cons_carts)
            self.cons_carts.append([])
        return cart_id

    def add_to_cart(self, cart_id, product):
        """
        Adds a product to cart.

        @type cart_id: Int
        @param cart_id: the id of the cart

        @type product: Product
        @param product: the product to be added

        @returns True or False. If the caller receives False, it should wait and then try again
        """
        with self.consumer_lock:
            # going through every list in the producer queue
            for producer_list in self.prod_queue:
                for product_in_list in producer_list:
                    if product_in_list['product'] == product:
                        # placing the product in the cart
                        self.cons_carts[cart_id].append(product_in_list)
                        # removing the product from the producer list
                        producer_list.remove(product_in_list)
                        return True
        return False

    def remove_from_cart(self, cart_id, product):
        """
        Removes a product from cart.

        @type cart_id: Int
        @param cart_id: id cart

        @type product: Product
        @param product: the product to remove from cart
        """
        with self.consumer_lock:
            for product_in_cart in self.cons_carts[cart_id]:
                if product_in_cart['product'] == product:
                    # making the product available again
                    self.prod_queue[product_in_cart['producer']].append(product_in_cart)
                    # removing from cart
                    self.cons_carts[cart_id].remove(product_in_cart)
                    break

    def place_order(self, cart_id):
        """
        Return a list with all the products in the cart.

        @type cart_id: Int
        @param cart_id: id cart
        """
        return [d['product'] for d in self.cons_carts[cart_id]]
