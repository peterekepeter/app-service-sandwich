#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <map>
#include <unordered_map>
#include "../AppServiceSandwichSrc/ResourceManager.hpp"

namespace Test
{
	TEST_CLASS(ResourceManagerTest)
	{
	public:

		/*
		* TEST CASE: ManagerTest
		*
		* a resource needs to be loaded
		* this test checks basic resource manager functionality
		*/

		class Brick : public ManagedResource
		{
		public:
			int size;

			explicit Brick(int size)
				: size(size)
			{
			}
		};

		class BrickFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath& path, ResourceManagerLocation& manager) override {
				return new Brick(42);
			}
		};

		TEST_METHOD(ManagerTest)
		{
			ResourceManager manager;
			BrickFactory brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);
			auto brick = manager.Require<Brick>("resources/red.txt");
			Assert::IsTrue(brick->size == 42);
		}

		/*
		* TEST CASE: ManagerSameResource
		*
		* the same resource is required twice
		* the test makes sure that the resource manager maps them to the same in memory object
		*/

		TEST_METHOD(ManagerSameResource)
		{
			auto path = "r.txt";
			ResourceManager manager;
			BrickFactory brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);
			auto a = manager.Require<Brick>(path);
			auto b = manager.Require<Brick>(path);
			Assert::IsTrue(&(a->size) == &(b->size));
		}

		/*
		* TEST CASE: ManagerChainedDependencyTest
		*
		* the required resource is built out of other stuff
		* this test case is here to ensure that we have access to resmanager when creating a resource
		*/

		class Wall : public ManagedResource
		{
		public:
			ResourcePtr<Brick> brickA;
			ResourcePtr<Brick> brickB;
			Wall(ResourcePtr<Brick> a, ResourcePtr<Brick> b) : brickA(a), brickB(b)
			{

			};
			int GetSum()
			{
				return brickA->size + brickB->size;
			}
		};

		class WallFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath& path, ResourceManagerLocation& manager) override {
				auto a = manager.Require<Brick>("bricks/a.txt");
				auto b = manager.Require<Brick>("bricks/b.txt");
				return new Wall(a, b);
			};
		};

		TEST_METHOD(ManagerChainedDependencyTest)
		{
			ResourceManager manager;
			manager.RegisterFactory<Wall, WallFactory>();
			manager.RegisterFactory<Brick, BrickFactory>();
			auto p = manager.Require<Wall>("resources/test/wall.txt");
			Assert::IsTrue(p->GetSum() == 2 * 42);
		}

		/*
		 * TEST CASE: ManagerReloadTest
		 *
		 * this test makes sure that the reload function works corretly within the resource manager
		 * check if the method NotifyResourceChange retriggers file loading
		 */

		class TestItem : public ManagedResource
		{
		public:
			int id;

			explicit TestItem(int id)
				: id(id)
			{
			}
		};

		class TestItemLoader : public ResourceFactory
		{
		public:
			int counter = 0;
			ManagedResource* operator()(const ResourcePath&, ResourceManagerLocation&) override
			{
				return new TestItem(++counter);
			}
		};

		TEST_METHOD(ManagerReloadTest)
		{
			ResourceManager manager;
			TestItemLoader loader;
			manager.RegisterFactory<TestItem>(loader);
			auto item = manager.Require<TestItem>("test.txt");
			Assert::IsTrue(item->id == 1);
			manager.NotifyResourceChange("test.txt");
			Assert::IsTrue(item->id == 2);
		}

		/*
		* TEST CASE: VirtualDerivedTypeTest
		*
		* the required resource is built using multiple inheritnace
		* this can cause some errors with pointers if the resman is badly implemented
		* the test checks whether data gets corrupted or not if multiple inheritance is used
		*/

		class InterfaceA
		{
		public:
			int x;

			explicit InterfaceA(int x)
				: x(x)
			{
			}

			virtual ~InterfaceA() {};
		};

		class InterfaceB
		{
		public:
			explicit InterfaceB(float y)
				: y(y)
			{
			}

			float y;

			virtual ~InterfaceB() {};
		};

		class DerivedType : public InterfaceA, public InterfaceB, public ManagedResource
		{
		public:
			DerivedType(int x, float y)
				: InterfaceA(x),
				InterfaceB(y)
			{
			}
		};

		class DerivedTypeFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath&, ResourceManagerLocation&) override
			{
				return new DerivedType(1, 2.0);
			}
		};

		TEST_METHOD(VirtualDerivedTypeTest)
		{
			ResourceManager r;
			r.RegisterFactory<DerivedType, DerivedTypeFactory>();
			auto item = r.Require<DerivedType>("");
			Assert::IsTrue(item->x == 1);
			Assert::IsTrue(item->y == 2.0);
		}

		/*
		 * TEST CASE: RequireHierarchyAtFactory
		 *
		 * inside this test we try to load an object with resource manager
		 * and that object will load a child object during creation
		 * the test should verify whether the cild object is loaded with relative path
		 */

		class ItemListConfig : public ManagedResource
		{
		public:
			ItemListConfig()
			{

			}
		};

		class ItemListConfigFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath& path, ResourceManagerLocation& resman) override
			{
				Assert::IsTrue(path == static_cast<ResourcePath>("./database/list/config.json"));
				return new ItemListConfig();
			}
		};

		class ItemList : public ManagedResource
		{
		public:
			ItemList(ResourcePtr<ItemListConfig> config)
			{
			}
		};

		class ItemListFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath&, ResourceManagerLocation& resman) override
			{
				return new ItemList(resman.Require<ItemListConfig>("./config.json"));
			}
		};

		TEST_METHOD(RequireHierarchyAtFactory)
		{
			ResourceManager r;
			r.RegisterFactory<ItemList, ItemListFactory>();
			r.RegisterFactory<ItemListConfig, ItemListConfigFactory>();
			auto result = r.Require<ItemList>("./database/list/data.txt");
		}

		/*
		* TEST CASE: RequireHierarchyAtRuntime
		*
		* inside this test we load an object and that object will request another object during runtime
		* this is a lazy loading features that can be used with resource manager
		* if the requested child is done with a relative path, it should be combined with the path of the parent
		*/

		class HierarchyItem : public ManagedResource
		{
		public:
			void ExecuteParent()
			{
				resourceManager.Require<HierarchyItem>("child.txt");
			}
		};


		class HierarchyItemFactory : public ResourceFactory
		{
			int counter = 0;
		public:
			ManagedResource* operator()(const ResourcePath& path, ResourceManagerLocation& resman) override
			{
				if (counter == 0) Assert::IsTrue(path == static_cast<ResourcePath>("./data/parent.txt"));
				if (counter == 1) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child.txt"));
				counter++;
				return new HierarchyItem();
			}
		};

		TEST_METHOD(RequireHierarchyAtRuntime)
		{
			ResourceManager r;
			HierarchyItemFactory factory;
			r.RegisterFactory<HierarchyItem>(factory);
			auto parent = r.Require<HierarchyItem>("./data/parent.txt");
			parent->ExecuteParent();

		}

		/*
		* TEST CASE: RequireHierarchyExternally
		*
		* require can be called from any managed resource from anywhere
		* it's reference to the resmanager is public
		* this method is not recomended, but it's avaiable to simplify certain usage scenarios
		*/

		class HierarchyItem2 : public ManagedResource
		{

		};

		class HierarchyItemFactory2 : public ResourceFactory
		{
			int counter = 0;
		public:
			ManagedResource* operator()(const ResourcePath& path, ResourceManagerLocation& resman) override
			{
				if (counter == 0) Assert::IsTrue(path == static_cast<ResourcePath>("./data/parent.txt"));
				if (counter == 1) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child.txt"));
				if (counter == 2) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child/items.txt"));
				if (counter == 3) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child/items.cfg"));
				if (counter == 4) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child/item/1.bin"));
				if (counter == 5) Assert::IsTrue(path == static_cast<ResourcePath>("./data/child/item/2.bin"));
				counter++;
				return new HierarchyItem2();
			}
		};

		TEST_METHOD(RequireHierarchyExternally)
		{
			ResourceManager r;
			HierarchyItemFactory2 factory;
			r.RegisterFactory<HierarchyItem2>(factory);
			auto parent = r.Require<HierarchyItem2>("data/parent.txt");

			// externally require from managed resource's perspective
			auto child = parent->resourceManager.Require<HierarchyItem2>("child.txt");
			auto child_items = child->resourceManager.Require<HierarchyItem2>("child/items.txt");
			auto items_cfg = child_items->resourceManager.Require<HierarchyItem2>("items.cfg");
			auto item_1 = items_cfg->resourceManager.Require<HierarchyItem2>("item/1.bin");
			auto item_2 = item_1->resourceManager.Require<HierarchyItem2>("2.bin");

		}

		/*
		* TEST CASE: ResourcePtrIsNull
		*
		* check if we can test if resource pointer is null or not
		*/

		TEST_METHOD(ResourcePtrIsNull)
		{
			ResourceManager manager;
			BrickFactory brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);
			ResourcePtr<Brick> brick;
			Assert::IsFalse(brick.IsLoaded());
			Assert::IsTrue(brick.operator->() == nullptr);
			brick = manager.Require<Brick>("resources/red.txt");
			Assert::IsTrue(brick.IsLoaded());
			Assert::IsFalse(brick.operator->() == nullptr);
		}

		/*
		 * TEST CASE: ResourcePtrAssignNull
		 * 
		 * working with nullptr
		 */
		TEST_METHOD(ResourcePtrAssignNull)
		{
			// init
			ResourceManager manager;
			BrickFactory brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);

			// get brick
			auto brick = manager.Require<Brick>("test");
			Assert::IsTrue(brick != nullptr); // should have value
			Assert::IsTrue(nullptr != brick); // should have value
			brick = nullptr; // null asignment
			Assert::IsTrue(brick == nullptr); // lost value
			Assert::IsTrue(nullptr == brick); // lost value

			// get brick2, same test but with methods
			auto brick2 = manager.Require<Brick>("test");
			Assert::IsTrue(brick2.IsLoaded()); // should have value
			brick2.Clear(); // null asignment
			Assert::IsFalse(brick2.IsLoaded()); // lost value
		}

		/*
		 * TEST CASE: FactoryFailsAndReturnsNull
		 *
		 * the case when the factory fails to build the resource
		 */
		class BrickFailFactory : public ResourceFactory
		{
		public:
			ManagedResource* operator()(const ResourcePath& resourcePath, ResourceManagerLocation& resourceManager) override {
				return nullptr;
			}
		};

		TEST_METHOD(FactoryFailsAndReturnsNull)
		{
			// init
			ResourceManager manager;
			BrickFailFactory brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);

			// get brick
			ResourcePtr<Brick> brick;
			Assert::IsTrue(brick.IsNull());
			Assert::IsTrue(brick.IsNotLoaded());
			brick = manager.Require<Brick>("test");
			Assert::IsTrue(brick.IsNotNull());
			Assert::IsTrue(brick.IsNotLoaded());
		}

		/*
		 * TEST CASE: FactoryFailsButReloadWorks
		 */
		class BrickFailFactoryReload : public ResourceFactory
		{
			int counter = 0;
		public:
			ManagedResource* operator()(const ResourcePath& resourcePath, ResourceManagerLocation& resourceManager) override {
				Brick* brick = nullptr;
				if (counter>0)
				{
					brick = new Brick(42);
				}
				counter++;
				return brick;
			}
		};


		TEST_METHOD(FactoryFailsButReloadWorks)
		{
			// init
			ResourceManager manager;
			BrickFailFactoryReload brick_factory;
			manager.RegisterFactory<Brick>(brick_factory);

			// get brick
			auto brick = manager.Require<Brick>("test");
			Assert::IsTrue(brick.IsNotNull());
			Assert::IsTrue(brick.IsNotLoaded());

			// on reload it should be properly loaded
			manager.NotifyResourceChange("test");
			Assert::IsTrue(brick.IsNotNull());
			Assert::IsTrue(brick.IsLoaded());
		}
	};
}