////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Реализация классов красно-черного дерева
/// \author    Sergey Shershakov
/// \version   0.1.0
/// \date      01.05.2017
///            This is a part of the course "Algorithms and Data Structures" 
///            provided by  the School of Software Engineering of the Faculty 
///            of Computer Science at the Higher School of Economics.
///
/// "Реализация" (шаблонов) методов, описанных в файле rbtree.h
///
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>        // std::invalid_argument


namespace xi {


//==============================================================================
// class RBTree::node
//==============================================================================

template <typename Element, typename Compar >
RBTree<Element, Compar>::Node::~Node()
{
    if (_left)
        delete _left;
    if (_right)
        delete _right;
}



template <typename Element, typename Compar>
typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setLeft(Node* lf)
{
    // предупреждаем повторное присвоение
    if (_left == lf)
        return nullptr;

    // если новый левый — действительный элемент
    if (lf)
    {
        // если у него был родитель
        if (lf->_parent)
        {
            // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
            if (lf->_parent->_left == lf)
                lf->_parent->_left = nullptr;
            else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                lf->_parent->_right = nullptr;      
        }

        // задаем нового родителя
        lf->_parent = this;
    }

    // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
    Node* prevLeft = _left;
    _left = lf;

    if (prevLeft)
        prevLeft->_parent = nullptr;

    return prevLeft;
}


template <typename Element, typename Compar>
typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setRight(Node* rg)
{
    // предупреждаем повторное присвоение
    if (_right == rg)
        return nullptr;

    // если новый правый — действительный элемент
    if (rg)
    {
        // если у него был родитель
        if (rg->_parent)
        {
            // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
            if (rg->_parent->_left == rg)
                rg->_parent->_left = nullptr;
            else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                rg->_parent->_right = nullptr;
        }

        // задаем нового родителя
        rg->_parent = this;
    }

    // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
    Node* prevRight = _right;
    _right = rg;

    if (prevRight)
        prevRight->_parent = nullptr;

    return prevRight;
}


//==============================================================================
// class RBTree
//==============================================================================

template <typename Element, typename Compar >
RBTree<Element, Compar>::RBTree()
{
    _root = nullptr;
    _dumper = nullptr;
}

template <typename Element, typename Compar >
RBTree<Element, Compar>::~RBTree()
{
    // грохаем пока что всех через корень
    if (_root)
        delete _root;
}


template <typename Element, typename Compar >
void RBTree<Element, Compar>::deleteNode(Node* nd)
{
    // если переданный узел не существует, просто ничего не делаем, т.к. в вызывающем проверок нет
    if (nd == nullptr)
        return;

    // потомков убьет в деструкторе
    delete nd;
}


template <typename Element, typename Compar >
void RBTree<Element, Compar>::insert(const Element& key)
{
    // этот метод можно оставить студентам целиком
    Node* newNode = insertNewBstEl(key);

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_INS, this, newNode);

    rebalance(newNode);

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_INSERT, this, newNode);

}




template <typename Element, typename Compar>
const typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::find(const Element& key)
{
    Node* curr = _root;

    while (curr != nullptr)
    {
        if (key < curr->getKey())
            curr = curr->getChild(true);
        else if (key > curr->getKey())
            curr = curr->getChild(false);
        else
            return curr;
    }
    return nullptr;
}

template <typename Element, typename Compar >
typename RBTree<Element, Compar>::Node*RBTree<Element, Compar>::insertNewBstEl(const Element& key)
{
    Node* y = nullptr;
    Node* x = _root;

    // Ищем место для нового узла
    while (x != nullptr)
    {
        y = x;
        if (key < x->getKey())
            x = x->getChild(true);
        else if (key > x->getKey())
            x = x->getChild(false);
        else
            throw std::invalid_argument("The element with the same key is already placed");
    }

    // Создаём новый узел
    Node* newNode = new Node(key, nullptr, nullptr, nullptr, RED);

    // Вставляем новый узел на найденное место
    if (y == nullptr) // Если дерево было пустое и новый узел стал корнем
    {
        _root = newNode;
        newNode->setBlack();
    }
    else
    {
        if (key < y->getKey())
            y->setLeft(newNode);
        else
            y->setRight(newNode);
    }

    return newNode;
}


template <typename Element, typename Compar >
typename RBTree<Element, Compar>::Node*RBTree<Element, Compar>::rebalanceDUG(Node* nd)
{
    // В методе оставлены некоторые важные комментарии/snippet-ы


    // попадание в этот метод уже означает, что папа есть (а вот про дедушку пока не известно)
    //...

    Node *uncle = nd->_parent->_parent->getChild(
            !nd->_parent->isLeftChild()); // для левого случая нужен правый дядя и наоборот.

    // если дядя такой же красный, как сам нод и его папа...
    if (uncle != nullptr && uncle->isRed()) {
        // дядю и папу красим в черное
        // а дедушку — в коммунистические цвета
        uncle->setBlack();
        nd->_parent->setBlack();
        uncle->_parent->setRed();

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

        // теперь чередование цветов "узел-папа-дедушка-дядя" — К-Ч-К-Ч, но надо разобраться, что там
        // с дедушкой и его предками, поэтому продолжим с дедушкой
        nd = uncle->_parent;
    }
    // дядя черный
    // смотрим, является ли узел "правильно-правым" у папочки
    else// для левого случая нужен правый узел, поэтом отрицание
    {                                               // CASE2 в действии
        if (nd->_parent->isLeftChild() && nd->isRightChild())
        {
            nd = nd->_parent;
            rotLeft(nd);
        }
        else if (nd->_parent->isRightChild() && nd->isLeftChild())
        {
            nd = nd->_parent;
            rotRight(nd);
        }
        // ... при вращении будет вызвано отладочное событие

        nd->_parent->setBlack();

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);


        // деда в красный

        nd->_parent->_parent->setRed();

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

        if (nd->_parent->isLeftChild())
            rotRight(nd->_parent->_parent);
        else
            rotLeft(nd->_parent->_parent);

    }
    return nd;
}


template <typename Element, typename Compar >
void RBTree<Element, Compar>::rebalance(Node* nd)
{
    if (nd->isBlack())
        return;

    // пока папа — цвета пионерского галстука, действуем
    while (nd->isDaddyRed())
    {
        // локальная перебалансировка семейства "папа, дядя, дедушка" и повторная проверка
        nd = rebalanceDUG(nd);
    }

    _root->setBlack();
}



template <typename Element, typename Compar>
void RBTree<Element, Compar>::rotLeft(typename RBTree<Element, Compar>::Node* nd)
{

    // правый потомок, который станет после левого поворота "выше"
    Node* y = nd->_right;
    
    if (!y)
        throw std::invalid_argument("Can't rotate left since the right child is nil");

    nd->setRight(y->getChild(true));

    if (y->getChild(true) != nullptr)
        y->getChild(true)->_parent = nd;


    if (nd->_parent == nullptr)
        _root = y;

    else if (nd->isLeftChild())
        nd->_parent->setLeft(y);

    else
        nd->_parent->setRight(y);

    y->setLeft(nd);
    nd->_parent = y;


    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_LROT, this, nd);
}



template <typename Element, typename Compar>
void RBTree<Element, Compar>::rotRight(typename RBTree<Element, Compar>::Node* nd)
{

    Node* y = nd->_left;
    if (!y)
        throw std::invalid_argument("Can't rotate right since the left child is nil");

    nd->setLeft(y->getChild(false));
    if (y->getChild(false) != nullptr)
        y->getChild(false)->_parent = nd;

    if (nd->_parent == nullptr)
        _root = y;

    else if (nd->isLeftChild())
        nd->_parent->setLeft(y);

    else
        nd->_parent->setRight(y);

    y->setRight(nd);
    nd->_parent = y;

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RROT, this, nd);
}
template <typename Element, typename Compar >
void RBTree<Element, Compar>::remove(const Element& key)
{
    const Node *x = find(key);
    Node *nd = const_cast<Node*>(x);
    Node *s;
    while (nd != _root && nd->isBlack())
    {
        if (nd == nd->_parent->_left)
        {
            s = nd->_parent->_right;
            if (s->isRed())
            {
                s->setBlack();
                nd->_parent->setRed();
                rotLeft(nd->_parent);
                s = nd->_parent->_right;
            }
            if (s->_left->isBlack() && s->_right->isBlack())
            {
                s->setRed();
                nd = nd->_parent;
            }
            else
            {
                if (s->_right->isBlack())
                {
                    s->_left->setBlack();
                    s->setRed();
                    rotRight(s);
                    s = nd->_parent->_right;
                }
                s->_color = nd->_parent->_color;
                nd->_parent->isBlack();
                s->_right->isBlack();
                rotLeft(nd->_parent);
                nd = _root;
            }
        }
        else
        {
            s = nd->_parent->_left;
            if (s->isRed())
            {
                s->setBlack();
                nd->_parent->setRed();
                rotRight(nd->_parent);
                s = nd->_parent->_left;
            }
            if (s->_right->isBlack() && s->_right->isBlack())
            {
                s->setRed();
                nd = nd->_parent;
            }
            else
            {
                if (s->_left->isBlack())
                {
                    s->_right->setBlack();
                    s->setRed();
                    rotLeft(s);
                    s = nd->_parent->_left;
                }
                s->_color = nd->_parent->_color;
                nd->_parent->setBlack();
                s->_left->setBlack();
                rotRight(nd->_parent);
                nd = _root;
            }
        }
    }
    nd->setBlack();

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RROT, this, nd);
}

} // namespace xi

