#include "acutest.hpp"
#include "entry.hpp"

void test_create(void) {
    Entry* e = new Entry("abcd", "1234");

    TEST_ASSERT(e != NULL);
    TEST_ASSERT(e->get_page_title() == "abcd");
    TEST_ASSERT(e->get_id() == "1234");
    
    delete e;
}

void test_merge(void) {
    Entry* e1 = new Entry("abcd", "1234");
    Entry* e2 = new Entry("efgh", "5678");

    e1->merge(e2);
    TEST_ASSERT(e1->clique == e2->clique);
    TEST_ASSERT(e1->clique->pop()->data->get_hashvalue() == e2->get_hashvalue());
    TEST_ASSERT(e1->clique->pop()->data->get_hashvalue() == e1->get_hashvalue());
    
    delete e1,e2;
}

void test_complicated_merge(void) {
    Entry *e1 = new Entry("e", "1");
    Entry *e2 = new Entry("e", "2");
    Entry *e3 = new Entry("e", "3");
    Entry *e4 = new Entry("e", "4");
    Entry *e5 = new Entry("e", "5");
    Entry *e6 = new Entry("e", "6");

    e1->merge(e2);
    e2->merge(e1);
    e1->merge(e3);
    e4->merge(e5);
    e4->merge(e6);
    e4->merge(e4);
    e1->merge(e4);
    e1->merge(e2);

    TEST_ASSERT(e1->clique == e2->clique
                && e2->clique == e3->clique
                && e3->clique == e4->clique
                && e4->clique == e5->clique
                && e5->clique == e6->clique);
    
    Clique* c = e1->clique;
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e4->get_hashvalue());
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e5->get_hashvalue());
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e6->get_hashvalue());
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e3->get_hashvalue());
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e2->get_hashvalue());
    TEST_ASSERT(c->pop()->data->get_hashvalue() == e1->get_hashvalue());
    
    
    delete e1,e2,e3,e4,e5,e6;
}

TEST_LIST = {
	{ "entry_create", test_create },
	{ "entry_merge", test_merge },
	{ "entry_complicated_merge", test_complicated_merge },
	{ NULL, NULL }
};