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

namespace SchoolManager\Objects\Traits;

trait Mail
{
    /**
     * @var string $mail
     */
    private $mail;

    /**
     * @param  string $mail E-Mail of student.
     * @param  bool $verify Only set mail property if
     *                      $mail parameter could be verified.
     *
     * @return bool true if input was valid, false if invalid.
     */
    public function setMail(string $mail, bool $verify = true): bool
    {
        if (!$verify) {
            $this->mail = $mail;
            return true;
        }

        if (filter_var($mail, FILTER_VALIDATE_EMAIL)) {
            $this->mail = $mail;
            return true;
        } else {
            return false;
        }
    }

    /**
     * @return string E-Mail address of user.
     */
    public function getMail(): string
    {
        return is_null($this->mail) ? "" : $this->mail;
    }
}
