
## JSF

Структура JSF приложения

- JSP или XHTML страницы, содержащие компоненты GUi

- Библиотека тегов - в них лежат доступные компоненты(теги) JSF

- Управляемые бины

- Дополнительные объекты (компоненты, конвертеры и валидаторы)

- Дополнительные теги

- Конфигурация - faces-config.xml (опционально, или аннотации)

- Дескриптор развертывания - web.xml

---

бины управляются контенером - JSF runtime, не программистом

MVC-модель JSF

FacesServlet (как работает)

конфигурация FacesServlet в web.xml

Страницы и компоненты UI

- Интерфейс строится из компонентов
- Компоненты расположены на Facelets-шаблонах или JSP страницах
- Компоненты реализуют интерфейс javax.faces.component.UIComponent
- Можно создавать собственные компоненты
- Компоненты на странице объединены в древовидную структуру - представление
- Корневым элементов представления является экземпляр класса javax.faces.component.UIViewRoot.

Пример:

```xhtml
<? xml version="1.0" encoding="UTF-8"?>
<! DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
    xmlns:f="http://java.sun.com/jsf/core"
    xmlns:h="http://java.sun.com/jsf/html">
<h:body>
    <h3>JSF 2.0 + Ajax Hello World Example</h3>
    <h:form>
        <h:inputText id="name" value="#{helloBean.name}"></h:inputText>
        <h:commandButton value="Welcome Me">
            <f:ajax execute="name" render="output" />
        </h:commandButton>
        <h2>
            <h:outputText id="output" value="#{helloBean.sayWelcome}" />
        </h2>
    </h:form>
</h:body>
</html>
```

теги обычного html, подключен без тега, просто xmlns

    xmlns="http://www.w3.org/1999/xhtml"

дополнительные namespaces jsf/core под переменной f

    xmlns:f="http://java.sun.com/jsf/core"

дополнительные namespaces jsf/html под переменной h

    xmlns:h="http://java.sun.com/jsf/html">


есть клиент, где работает бразур и он не видит всех jsf теги, браузер на вход получает html, из xhtml FacesServlet преобразует в html путем цикла из xml (xhtml) переводит в html 

на каждой итерации запроса, FacesServlet занимается синхронизщацией состояния представления xhtml и DOM моделью html на клиенте

приложение постоянно крутится в цикле и синхронизирует состояние


## Навигация между страницами JSF

Есть три способа 

1. Правила задаются в файле faces-config.xml:

```xml
<navigation-rule>
    <from-view-id>/pages/a.xhtml</from-view-id>
    <navigation-case>
        <from-outcome>doRedirect</from-outcome>
        <to-view-id>/pages/b.xhtml</to-view-id>
    </navigation-case>
    <navigation-case>
        <to-view-id>/pages/c.xhtml</to-view-id>
    </navigation-case>
</navigation-rule>
```

То есть мы на странице a.xhtml вызываем doRedirect и переходим на страницу b.xhtml

Этим способом мы можем задать свои URL, а не example.xhtml

Пример перенаправления на другую страницу:

```xml
<h:commandButton id="submit" action="doRedirect" value="Submit" />
```

2. Правила задаются в методе action:

```xml
<h:commandButton id="submit" action="page/b.xhtml" value="Submit" />
```

## Управляемые бины

- Содержат параметры и методы для обработки данных с компонентов

- Используются для обработки событий UI и валидации данных

- Жизненным циклом управляет JSF Runtime Envronment

- Доступ из JSF-страниц осуществляется с помощью элементов EL

- Конфигурация задаётся в faces-config.xml (JSF 1.Х), либо с помощью аннотаций (JSF 2.0)


Пример:

```java
import javax.faces.bean.ManagedBean;
import javax.faces.bean. SessionScoped;
import java.io.Serializable;
@ManagedBean
@SessionScoped
public class HelloBean implements Serializable {
    private static final long serialVersionUID = 1L;
    private String name;
    public String getName() {
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public String getSayWelcome(){
        if("".equals(name) || name == null){
            return "";
        } else {
            return "Ajax message : Welcome " + name;
        }
    }
}
```

@ManagedBean - управляемый бин

@SessionScoped - область видимости

Конекст (scope) управляемых бинов

Задаётся через faces-config.xml или с помощью аннотации.

6 вариантов конфигурации:

@NoneScoped - контекст не определён, жизненным циклом управляют другие бины.

@RequestScoped (применяется по умолчанию) - контекст - запрос.

@ViewScoped (JSF 2.0) - контекст - страница.

@SessionScoped - контекст - сессия.

@ApplicationScoped - контекст - приложение.

@CustomScoped (JSF 2.0) - бин сохраняется в Мар; программист сам управляет его жизненным циклом.

Конфигурация управляемых бинов

Способ 1 - через faces-config.xml:

```xml
<managed-bean>
    <managed-bean-name>customer</managed-bean-name>
    <managed-bean-class>CustomerBean</managed-bean-class>
    <managed-bean-scope>request</managed-bean-scope>
    <managed-property>
        <property-name>areaCode</property-name>
        <value>#{initParam.defaultAreaCode}</value>
    </managed-property>
</managed-bean>
```

Способ 2 (JSF 2.0) - с помощью аннотаций:

```java
@ManagedBean(name="customer")
@RequestScoped
public class CustomerBean {
    ...
    @ManagedProperty(value="#{initParam.defaultAreaCode}" name="areaCode")
    private String areaCode;
    ..
}
```

Доступ к управляемым бинам со страц приложения

Осуществляется с помощью EL-выражений:

```xhtml
<h:inputText value="#{user.name}" validator="#{user.validate}" />
<h:inputText binding="#{user.nameField}" />
<h:commandButton action="#{user.save}" value="Save" />
```

