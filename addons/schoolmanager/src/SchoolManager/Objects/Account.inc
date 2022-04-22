<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2022 Daniel Teichmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

namespace SchoolManager\Objects;

// *Load* optional properties.
require_once "Traits/trait_mail.inc";
require_once "Traits/trait_gender.inc";
require_once "Traits/trait_birthday.inc";

/**
 * TODO: Write description…
 */
abstract class Account
{
    /** @var Utils Utils class. */
    public $utils;

    /** @var string GOsa² account UID. */
    public $uid;

    /** @var string GOsa² employee number. */
    public $employeeNumber;

    /** @var string GOsa² account DN (Should include UID). */
    public $dn;

    /** @var string First and middle name of user. */
    public $givenName;

    /** @var string Last name of user. */
    public $secondName;

    /** @var Array Status string-array. Shows import/archive/… status of account. */
    public $status;

    /** @var Array Actions string-array. Shows import/archive/… actions of account. */
    public $actions;

    /**
     * Interceptor for non-accessible properties.
     *
     * @param string $property
     * @return mixed
     */
    public function __get($property)
    {
        if (method_exists($this, "get$property")) {
            return call_user_func(array($this, "get$property"));
        } else {
            return $this->$property;
        }
    }

    /**
     * Dump contents of account into JS Console.
     *
     * @return void
     */
    public function dumpToConsole(): void
    {
        $this->utils->debug_to_console(get_class($this) . ": " . $this->__toString());

        $traits_to_dump = array(
            _("Birthday") => array(
                "SchoolManager\Objects\Traits\Birthday",
                $this->birthday
            ),
            _("Gender")   => array(
                "SchoolManager\Objects\Traits\Birthday",
                $this->gender
            ),
            _("E-Mail")   => array(
                "SchoolManager\Objects\Traits\Mail",
                $this->mail
            )
        );

        foreach ($traits_to_dump as $trait_displayname => $value) {
            $trait_path  = $value[0];
            $trait_value = $value[1];

            // Don't try to get properties which aren't even used by this account.
            $used_traits = class_uses($this);
            if (!in_array($trait_path, $used_traits, true)) {
                continue;
            }

            $this->utils->debug_to_console("\t" . $trait_displayname . ": " .
                (is_null($trait_value) ? "❌" : $trait_value));
        }
    }

    /**
     * Compares $acc_a and $acc_b.
     *
     * @param $cmp_attrs Attributes to compare.
     * @return ?array Null when equal otherwise return unequal attributes.
     */
    public static function compare(
        Account &$acc_a,
        Account &$acc_b,
        array $cmp_attrs = array("uid", "givenName", "secondName", "mail", "birthday"),
        ?string $unequal_prefix = null,
        bool $case_insentive = false
    ): ?array {
        if ($acc_a == $acc_b) {
            return null;
        }

        if (
            !$acc_a instanceof self ||
            !$acc_b instanceof self
        ) {
            return null;
        }

        $unequal = array();
        foreach ($cmp_attrs as $key => $attr) {
            $val_a = $acc_a->$attr;
            $val_b = $acc_b->$attr;

            if ($case_insentive) {
                if (is_string($val_a)) {
                    $val_a = strtolower($val_a);
                }
                if (is_string($val_b)) {
                    $val_b = strtolower($val_b);
                }
            }

            if ($val_a !== $val_b) {
                $unequal[] = $unequal_prefix ? ($unequal_prefix . $attr) : $attr;
            }
        }

        if (empty($unequal)) {
            // If 'null' is returned, objects regarding the given attributes are identical
            return null;
        } else {
            // Otherwise return those attribute names that did not give a match
            return $unequal;
        }
    }

    /**
     * Compares self and $other_acc.
     *
     * @return bool True if equal otherwise false.
     */
    public function isEqual(
        Account &$other_acc,
        array $cmp_attrs = array("uid", "givenName", "secondName", "mail", "birthday"),
        ?string $unequal_prefix = null,
        bool $case_insentive = false
    ): ?array {
        return Account::compare($this, $other_acc, $cmp_attrs, $unequal_prefix, $case_insentive);
    }


    /**
     * Every account must have a constructor.
     *
     * @param Utils $utils Utils class. Automagically loaded by GOsa².
     */
    public function __construct(
        &$utils,
        string $givenName,
        string $secondName
    ) {
        $this->utils      = &$utils;
        $this->givenName  = $givenName;
        $this->secondName = $secondName;
    }

    /**
     * For convience: Accounts should be Stringable.
     *
     * @return string Stringified account.
     *                For example: "<Jon, Doe> (jdoe)" or "<Jon, Doe>"
     */
    public function __toString(): string
    {
        $ret = "<"  . $this->givenName  . ", " . $this->secondName . ">" .
               ($this->uid ? (" ($this->uid)") : "");

        return $ret;
    }

    /**
     * Generate a new free UID for this account.
     *
     * @return bool true if successful, false if unsucessful.
     */
    public function generateUID(): bool
    {
        // Get "idGenerator" property from "core" class.
        $_genStr = $this->utils->config->get_cfg_value("core", "idGenerator");

        if ($_genStr != "") {
            // Use _attributes to generate a unique ID.
            $_attributes = array(
                'sn'        => $this->secondName,
                'givenName' => $this->givenName
            );
            $_genUids = gen_uids($_genStr, $_attributes);

            // If successfull, use the first UID we generated above.
            if (is_countable($_genUids) and (count($_genUids) >= 1)) {
                $this->uid = $_genUids[0];
            } else {
                msg_dialog::display(
                    _("ERROR"),
                    _("Failure during UID auto-generation for " . to_string($this) .
                      "! No list of possible UIDs returned."),
                    ERROR_DIALOG
                );

                return false;
            }
        } else {
            msg_dialog::display(
                _("ERROR"),
                _("We can only auto-generate UID strings if the idGenerator " .
                  "is configured!"),
                ERROR_DIALOG
            );

            return false;
        }

        return true;
    }
}
